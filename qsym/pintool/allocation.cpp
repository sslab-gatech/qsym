#include "allocation.h"
#include "pin.H"

namespace qsym {

#ifdef  HUGE_TLB
  const ADDRINT kMapFlags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB;
#else
  const ADDRINT kMapFlags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE;
#endif

void deallocPages(void* addr, size_t length) {
  if (munmap(addr, length) == -1)
    LOG_FATAL("munmap failed");
}

void* allocPages(size_t length, int prot) {
  void* ptr = mmap(NULL, length, prot, kMapFlags, -1, 0);
  if (ptr == MAP_FAILED) {
    LOG_FATAL("out of memory (allocPages)");
    return NULL;
  }
  else
    return ptr;
}

void* allocRWPages(size_t length) {
  return allocPages(length, PROT_READ | PROT_WRITE);
}

void* safeRealloc(void* addr, size_t size) {
  void* ptr = realloc(addr, size);
  if (ptr == NULL) {
    LOG_FATAL("out of memory (safeRealloc)");
    return NULL;
  }
  else
    return ptr;
}

void* safeMalloc(size_t size) {
  void* ptr = malloc(size);
  if (ptr == NULL)
    LOG_FATAL("out of memory (safeMalloc)");
  return ptr;
}

void* safeCalloc(size_t nmemb, size_t size) {
  void* ptr = calloc(nmemb, size);
  if (ptr == NULL)
    LOG_FATAL("out of memory (safeCalloc)");
  return ptr;
}

} // namespace qsym
