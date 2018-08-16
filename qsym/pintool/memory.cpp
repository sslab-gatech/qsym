#include "memory.h"

namespace qsym {

namespace {
  inline ADDRINT roundUp(ADDRINT num, ADDRINT multiple) {
    ADDRINT remainder = num % multiple;
    if (remainder == 0)
      return num;
    else
      return num + multiple - remainder;
  }

  inline ADDRINT roundDown(ADDRINT num, ADDRINT multiple) {
    return (num / multiple) * multiple;
  }

  inline ADDRINT roundPageUp(ADDRINT num) {
    return roundUp(num, kPageSize);
  }

  inline ADDRINT roundPageDown(ADDRINT num) {
    return roundDown(num, kPageSize);
  }

  void getVdso(ADDRINT *start_addr, ADDRINT *end_addr) {
    FILE	*fp		= NULL;
    char	maps_path[PATH_MAX];
    char	lbuf[kMapsEntryMax];

    *start_addr = *end_addr = 0;
    snprintf(maps_path, PATH_MAX, "/proc/%d/maps", PIN_GetPid());
    if ((fp = fopen(maps_path, "r")) == NULL)
      LOG_FATAL("failed to open" + std::string(maps_path) + "\n");

    while(!feof(fp)) {
      if (fgets(lbuf, sizeof(lbuf), fp) == NULL)
        LOG_FATAL("failed to read" + std::string(maps_path) + "\n");

      if (strstr(lbuf, "[vdso]") != NULL) {
        (void)sscanf(lbuf, "%p-%p %*s:4 %*x %*s:5 %*u%*s\n",
            (void**)start_addr, (void**)end_addr);
        break;
      }
    }

    (void)fclose(fp);
  }

} // anonymous namespace

Memory::Memory()
  : page_table_(),
  unmapped_page_(NULL),
  zero_page_(NULL),
  brk_page_(NULL),
  brk_start_(0),
  brk_end_(0),
  off_(0) {}

Memory::~Memory()
{
  if (unmapped_page_ != NULL)
  {
    deallocPages(unmapped_page_, kPageSize * sizeof(ExprRef));
  }

  if (zero_page_ != NULL)
  {
    deallocPages(unmapped_page_, kPageSize * sizeof(ExprRef));
  }
}

void Memory::initialize()
{
  unmapped_page_ = (ExprRef*) allocPages(
      kPageSize * sizeof(ExprRef),
      PROT_NONE);
  zero_page_ = (ExprRef*) allocPages(
      kPageSize * sizeof(ExprRef),
      PROT_READ);

  setupVdso();
}

void Memory::allocateStack(ADDRINT stack_start) {
  // NOTE: heuristically find stack_start
  // maybe it could cause bugs
  // need to find better way to find stack base
  stack_start = roundPageUp(stack_start) + kPageSize;
  mmap(stack_start - kStackSize, stack_start);
}

void Memory::mmap(ADDRINT start, ADDRINT end) {
  start = roundPageDown(start);
  end = roundPageDown(end);
  ADDRINT length = (end - start) + kPageSize;

  if (length <= 0)
    LOG_FATAL("negative length");

  ExprRef* page = (ExprRef*)allocRWPages(length * sizeof(ExprRef));

  for (ADDRINT i = addressToPageIndex(start), j = 0;
      i <= addressToPageIndex(end); i++, j++) {
    page_table_[i] = page + j * kPageSize;
  }
}

void Memory::mremap(ADDRINT old_start,
    size_t old_size,
    ADDRINT new_start,
    size_t new_size) {
  ADDRINT old_end = roundPageDown(old_start + old_size - 1);
  ADDRINT new_end = roundPageDown(new_start + new_size - 1);

  // XXX: pin2.14 does not support mremap
  ExprRef* old_page = getPage(old_start);
  ExprRef* new_page = (ExprRef*)allocRWPages(new_size * sizeof(ExprRef));
  memcpy(new_page, old_page, old_size * sizeof(ExprRef));
  deallocPages((void*)old_page, old_size * sizeof(ExprRef));

  for (ADDRINT i = addressToPageIndex(old_start), j = 0;
      i <= addressToPageIndex(old_end); i++, j++)
    page_table_[i] = unmapped_page_;

for (ADDRINT i = addressToPageIndex(new_start), j = 0;
      i <= addressToPageIndex(new_end); i++, j++)
    page_table_[i] = new_page;
}

void Memory::munmap(ADDRINT start, USIZE size) {
  ADDRINT end = roundPageDown(start + size - 1);
  start = roundPageUp(start);

  ExprRef* page = getPage(start);
  deallocPages((void*)page, size * sizeof(ExprRef));

  for (ADDRINT i = addressToPageIndex(start);
      i <= addressToPageIndex(end); i++)
    page_table_[i] = unmapped_page_;
}

void Memory::initializeBrk(ADDRINT addr) {
  if (brk_start_ == 0) {
    addr = roundPageUp(addr);
    brk_start_ = addr;
    brk_end_ = addr;
  }
}

void Memory::brk(ADDRINT addr) {
  ADDRINT start = brk_start_;
  ADDRINT end = roundPageUp(addr);
  brk_page_ = (ExprRef*)safeRealloc(brk_page_, (end - start) * sizeof(ExprRef));

  for (ADDRINT i = addressToPageIndex(start), j = 0;
      i <= addressToPageIndex(end - 1); i++, j++) {
      page_table_[i] = brk_page_ + j * kPageSize;
  }

  for (ADDRINT i = addressToPageIndex(end);
      i <= addressToPageIndex(brk_end_); i++) {
      page_table_[i] = unmapped_page_;
  }

  brk_end_ = end;
}

bool Memory::isUnmappedAddress(ADDRINT vaddr) {
  return getPage(vaddr) == unmapped_page_;
}

bool Memory::isReadable(ADDRINT vaddr, INT32 size) {
  ADDRINT end = vaddr + size;
  ADDRINT page = roundPageDown(vaddr);

  while (true) {
    if (page > end)
      break;
    if (isUnmappedAddress(page))
      return false;
    page += kPageSize;
  }
  return true;
}

void Memory::setupVdso() {
  ADDRINT start_addr, end_addr;
  getVdso(&start_addr, &end_addr);

  LOG_DEBUG("vdso: " + hexstr(start_addr) + "-" + hexstr(end_addr) + "\n");

  if (start_addr != 0) {
    for (ADDRINT i = addressToPageIndex(start_addr);
        i <= addressToPageIndex(end_addr); i++)
      page_table_[i] = zero_page_;
  }
}

} // namespace qsym
