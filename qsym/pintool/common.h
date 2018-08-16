#ifndef QSYM_COMMON_H_
#define QSYM_COMMON_H_

#include "pin.H"
#include "logging.h"
#include "compiler.h"
#include "allocation.h"
#include <llvm/ADT/APSInt.h>

#define EXPR_COMPLEX_LEVEL_THRESHOLD 4
#define XXH_STATIC_LINKING_ONLY
#include "third_party/xxhash/xxhash.h"


inline REG getAx(USIZE size) {
  switch(size) {
    case 1:
      return REG::REG_AL;
    case 2:
      return REG::REG_AX;
    case 4:
      return REG::REG_EAX;
#if __x86_64__
    case 8:
      return REG::REG_RAX;
#endif
    default:
      return REG_INVALID();
  }
}

inline REG getAx(REG r) {
  return getAx(REG_Size(r));
}

inline REG getDx(USIZE size) {
  switch(size) {
    case 1:
      return REG::REG_DL;
    case 2:
      return REG::REG_DX;
    case 4:
      return REG::REG_EDX;
#if __x86_64__
    case 8:
      return REG::REG_RDX;
#endif
    default:
      return REG_INVALID();
  }
}

inline REG getDx(REG r) {
  return getDx(REG_Size(r));
}

inline bool isInterestingReg(REG r) {
  if (REG_FullRegName(r) != r)
    return false;
  // TODO: REG_is_mm(r) can be added?
  // E: Register mm0 is NOT supported for PIN_GetContextReg/PIN_SetContextReg
  return REG_is_gr(r) ||  REG_is_xmm(r) || REG_is_ymm(r) || r == REG_INST_PTR;
}

inline INT32 getBitCount(INT32 x) {
  return __builtin_popcount(x);
}

inline ADDRINT toTruncValue(llvm::APInt val) {
  return val.trunc(sizeof(ADDRINT) * CHAR_BIT).getZExtValue();
}

inline ADDRINT getMask(ADDRINT size) {
  switch (size) {
    case 1:
      return 0xff;
    case 2:
      return 0xffff;
    case 4:
      return 0xffffffff;
#ifdef __x86_64__
    case 8:
      return 0xffffffffffffffff;
#endif
    default:
      CRASH();
      return -1;
  }
}

#endif
