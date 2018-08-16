#ifndef QSYM_MEMORY_H_
#define QSYM_MEMORY_H_

#include <string.h>
#include <unordered_map>

#include "allocation.h"
#include "call_stack_manager.h"
#include "compiler.h"
#include "expr_builder.h"
#include "trace.h"

namespace qsym {

const ADDRINT kPageShift = 12;
const ADDRINT kPageSize = (1U << kPageShift);
const ADDRINT kStackSize = (kPageSize << 11);
const ADDRINT kPageTableSize = (1U << 20);
const ADDRINT kUserStart = 0x00000000U;
const ADDRINT kUserEnd = 0xFFFFFFFFU;

#if 0
// in 64bit machine, stack size is different
const ADDRINT kUserEnd = 0xBFFFFFFFU;
const ADDRINT kKernStart = 0xC0000000U;
const ADDRINT kKernEnd = 0xFFFFFFFFU;
const ADDRINT kStackStart = kKernStart - kStackSize;
#endif

const ADDRINT kStackStart = - kStackSize;
const INT32   kMapsEntryMax = 128;
const ADDRINT kPageMask = (kPageSize - 1);

inline ADDRINT addressToPageIndex(ADDRINT addr) {
  return addr >> kPageShift;
}

inline ADDRINT pageIndexToAddress(ADDRINT index) {
  return index << kPageShift;
}

inline ADDRINT addressToOffset(ADDRINT addr) {
  return addr & kPageMask;
}

namespace {

inline llvm::APInt getMemValue(ADDRINT addr, INT32 size) {
  unsigned num_words = (size + sizeof(uint64_t) - 1) / sizeof(uint64_t);
  uint64_t value[num_words];
  PIN_SafeCopy(&value, (void*)addr, size);
  return llvm::APInt(llvm::APInt(size * 8, num_words, value));
}

inline ExprRef getMemValueExpr(ADDRINT addr, INT32 size) {
  llvm::APInt v = getMemValue(addr, size);
  return g_expr_builder->createConstant(v, size * CHAR_BIT);
}

} // namespace

class Memory {
public:
  Memory();
  ~Memory();

  void initialize();

  ADDRINT brk_start() { return brk_start_; }
  ADDRINT brk_end() { return brk_end_; }

  void allocateStack(ADDRINT stack_start);
  void mmap(ADDRINT, ADDRINT);
  void mremap(ADDRINT, size_t, ADDRINT, size_t);
  void munmap(ADDRINT, USIZE);
  void initializeBrk(ADDRINT);
  void brk(ADDRINT);
  bool isUnmappedAddress(ADDRINT);
  bool isReadable(ADDRINT vaddr, INT32 size);

  inline ExprRef getExprFromMemAlways(ADDRINT addr) {
    // if it is concrete, then do not return it
    // this symbol will be released by clearExprFromMem or
    // overwrite by setExprToMem
    ExprRef e = *getExprPtrFromMem(addr);
    return e;
  }

  inline ExprRef getExprFromMem(ADDRINT addr) {
    // if it is concrete, then do not return it
    // this symbol will be released by clearExprFromMem or
    // overwrite by setExprToMem
    return getExprFromMemAlways(addr);
  }

  inline ExprRef getExprFromMemAlways(ADDRINT addr, INT32 size) {
    if (!isSymbolicMem(addr, size))
      return NULL;

    UINT8 val[size];
    std::list<ExprRef> exprs;
    PIN_SafeCopy(val, (void*)addr, size);

    for (INT32 i = 0; i < size; i++) {
      ExprRef e_partial = getExprFromMem(addr + i);
      if (e_partial == NULL)
        e_partial = g_expr_builder->createConstant(val[i], 8);
      exprs.push_front(e_partial);
    }

    ExprRef e = g_expr_builder->createConcat(exprs);

    if (e != NULL && e->isConcrete()) {
      clearExprFromMem(addr, size);
      return NULL;
    }

#ifdef CONFIG_TRACE
    trace_getExprFromMem(e, addr, size);
#endif
    return e;
  }

  inline ExprRef getExprFromMem(ADDRINT addr, INT32 size) {
    return getExprFromMemAlways(addr, size);
  }

  inline void setExprToMem(ADDRINT addr, ExprRef e) {
    if (e == NULL) {
      clearExprFromMem(addr);
      return;
    }
    else {
      clearExprFromMem(addr);
      *getExprPtrFromMem(addr) = e;
    }
  }

  inline void setExprToMem(ADDRINT addr, INT32 size, ExprRef e) {
    if (e == NULL)
      clearExprFromMem(addr, size);
    else {
      for (INT32 i = 0; i < size; i++) {
        ExprRef new_expr = g_expr_builder->createExtract(e, i * 8, 8);
        setExprToMem(addr + i, new_expr);
      }
    }
  }

  inline void clearExprFromMem(ADDRINT addr) {
    ExprRef* ptr = getExprPtrFromMem(addr);
    ExprRef e = *ptr;
    if (e != NULL) {
      *ptr = NULL;
    }
  }

  inline void clearExprFromMem(ADDRINT addr, INT32 size) {
    for (INT32 i = 0; i < size; i++)
      clearExprFromMem(addr + i);
  }

  inline void makeExpr(ADDRINT addr, INT32 size) {
    LOG_DEBUG("makeExpr: addr=" + hexstr(addr)
        + ", size=" + hexstr(size) + "\n");
    for (INT32 i = 0; i < size; i++)
      makeExpr(addr + i);
  }

  inline void makeExpr(ADDRINT addr) {
    ExprRef e = g_expr_builder->createRead(off_++);
    setExprToMem(addr, e);
  }

  inline void lseek(ADDRINT off) {
    off_ = off;
  }

  inline ADDRINT tell()
  {
    return off_;
  }

protected:
  std::unordered_map<ADDRINT, ExprRef*> page_table_;
  ExprRef*  stack_page_;
  ExprRef*  unmapped_page_;
  ExprRef*  zero_page_;
  ExprRef*  brk_page_;
  ADDRINT brk_start_, brk_end_;
  ADDRINT off_;

  void setupPageTable();
  void setupVdso();

  inline ExprRef* getPage(ADDRINT addr) {
    auto it = page_table_.find(addressToPageIndex(addr));
    if (it == page_table_.end())
      return NULL;
    else
      return it->second;
  }

  inline ExprRef* getExprPtrFromMem(ADDRINT addr) {
    ExprRef* page = getPage(addr);
    // NOTE: this could happen due to ASLR
    // If we have better way to find stack address,
    // then this code could be removed
    if (page == NULL)
      return zero_page_;
    return &page[addressToOffset(addr)];
  }

  inline bool isSymbolicMem(ADDRINT addr, INT32 size) {
    for (INT32 i = 0; i < size; i++) {
      if (getExprFromMem(addr + i))
        return true;
    }
    return false;
  }
};
} // namespace qsym
#endif // QSYM_MEMORY_H_
