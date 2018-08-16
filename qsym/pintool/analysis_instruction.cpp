#include <cassert>
#include "analysis_instruction.h"
#include "instrument.h"

namespace qsym {

extern REG g_thread_context_reg;

#define IARG_THREAD_CONTEXT \
          IARG_REG_VALUE, \
          g_thread_context_reg

#define IARG_CPU_CONTEXT   \
          IARG_THREAD_CONTEXT, \
          IARG_CONTEXT

#define IARG_REG(r) \
          IARG_UINT32, \
          r

#define IARG_IMM(imm)           \
          IARG_ADDRINT,   \
          imm

#define IARG_ARG(arg) \
        IARG_UINT32, \
        arg

#define IARG_MEM(ins, addr_ty, size_ty) \
      IARG_UINT32, \
      INS_MemoryBaseReg(ins), \
      IARG_UINT32, \
      INS_MemoryIndexReg(ins), \
      addr_ty, \
      size_ty, \
      IARG_ADDRINT, \
      INS_MemoryDisplacement(ins), \
      IARG_UINT32, \
      INS_MemoryScale(ins)

#define IARG_MEM_WRITE(ins) \
      IARG_MEM(ins, IARG_MEMORYWRITE_EA, IARG_MEMORYWRITE_SIZE)

#define IARG_MEM_READ(ins)      \
      IARG_MEM(ins,             \
          IARG_MEMORYREAD_EA,   \
          IARG_MEMORYREAD_SIZE)

#define IARG_MEM_LEA(ins)       \
      IARG_MEM(ins,             \
          IARG_MEMORYREAD_EA)

// XXX: best?
#define GET_REG(ins, op_num) \
  ({              \
    REG _r = INS_OperandReg(ins, op_num);   \
    REG _full_reg = REG_FullRegName(_r);    \
    if (!isInterestingReg(_full_reg))       \
      return;                               \
    _r;                                     \
  })

namespace {

void getMemoryType(INS ins, INT32 i, INT32& count,
    IARG_TYPE& addr_ty, IARG_TYPE& size_ty) {
  if (INS_OperandRead(ins, i)) {
    if (count == 0) {
      addr_ty = IARG_MEMORYREAD_EA;
      size_ty = IARG_MEMORYREAD_SIZE;
      count += 1;
    }
    else {
      assert(count == 1);
      addr_ty = IARG_MEMORYREAD2_EA;
      size_ty = IARG_MEMORYREAD_SIZE;
    }
  }
  else {
    assert(INS_OperandWritten(ins, i));
    addr_ty = IARG_MEMORYWRITE_EA;
    size_ty = IARG_MEMORYWRITE_SIZE;
  }
}

void getMemoryType(bool write,
    IARG_TYPE& addr_ty,
    IARG_TYPE& size_ty) {
  if (write) {
    addr_ty = IARG_MEMORYWRITE_EA;
    size_ty = IARG_MEMORYWRITE_SIZE;
  }
  else {
    addr_ty = IARG_MEMORYREAD_EA;
    size_ty = IARG_MEMORYREAD_SIZE;
  }
}

static void
analyzeUnary(
    INS ins,
    bool write,
    AFUNPTR instrument_r,
    AFUNPTR instrument_m) {
  if (INS_OperandIsReg(ins, OP_0)) {
    QSYM_ASSERT(instrument_r != NULL);
    REG dst = GET_REG(ins, OP_0);

    INS_InsertCall(ins,
        IPOINT_BEFORE,
        instrument_r,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_REG(dst),
        IARG_END);
  }
  else if (INS_OperandIsMemory(ins, OP_0)) {
    QSYM_ASSERT(instrument_m != NULL);

    IARG_TYPE addr_ty, size_ty;
    getMemoryType(write, addr_ty, size_ty);

    INS_InsertCall(ins,
        IPOINT_BEFORE,
        instrument_m,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_MEM(ins, addr_ty, size_ty),
        IARG_END);
  }
  else
    UNREACHABLE();
}

static void
analyzeUnary(
    INS ins,
    bool write,
    UINT arg,
    AFUNPTR instrument_r,
    AFUNPTR instrument_m) {
  if (INS_OperandIsReg(ins, OP_0)) {
    QSYM_ASSERT(instrument_r != NULL);
    REG dst = INS_OperandReg(ins, OP_0);

    INS_InsertCall(ins,
        IPOINT_BEFORE,
        instrument_r,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_REG(dst),
        IARG_ARG(arg),
        IARG_END);
  }
  else if (INS_OperandIsMemory(ins, OP_0)) {
    QSYM_ASSERT(instrument_m != NULL);

    IARG_TYPE addr_ty, size_ty;
    getMemoryType(write, addr_ty, size_ty);

    INS_InsertCall(ins,
        IPOINT_BEFORE,
        instrument_m,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_MEM(ins, addr_ty, size_ty),
        IARG_ARG(arg),
        IARG_END);
  }
  else
    UNREACHABLE();
}

static void
analyzeUnary(
    INS ins,
    bool write,
    UINT arg1,
    UINT arg2,
    AFUNPTR instrument_r,
    AFUNPTR instrument_m) {
  if (INS_OperandIsReg(ins, OP_0)) {
    QSYM_ASSERT(instrument_r != NULL);
    REG dst = INS_OperandReg(ins, OP_0);

    INS_InsertCall(ins,
        IPOINT_BEFORE,
        instrument_r,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_REG(dst),
        IARG_ARG(arg1),
        IARG_ARG(arg2),
        IARG_END);
  }
  else if (INS_OperandIsMemory(ins, OP_0)) {
    QSYM_ASSERT(instrument_m != NULL);

    IARG_TYPE addr_ty, size_ty;
    getMemoryType(write, addr_ty, size_ty);

    INS_InsertCall(ins,
        IPOINT_BEFORE,
        instrument_m,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_MEM(ins, addr_ty, size_ty),
        IARG_ARG(arg1),
        IARG_ARG(arg2),
        IARG_END);
  }
  else
    UNREACHABLE();
}

static void
analyzeBinary(
    INS ins,
    bool write,
    AFUNPTR instrument_rr,
    AFUNPTR instrument_ri,
    AFUNPTR instrument_rm,
    AFUNPTR instrument_mr,
    AFUNPTR instrument_mi) {
  if (INS_OperandIsReg(ins, OP_0)) {
    REG dst = GET_REG(ins, OP_0);

    if (INS_OperandIsReg(ins, OP_1)) {
      QSYM_ASSERT(instrument_rr != NULL);

      REG src = GET_REG(ins, OP_1);
      INS_InsertCall(ins,
          IPOINT_BEFORE,
          instrument_rr,
          IARG_FAST_ANALYSIS_CALL,
          IARG_CPU_CONTEXT,
          IARG_REG(dst),
          IARG_REG(src),
          IARG_END);
    }
    else if (INS_OperandIsImmediate(ins, OP_1)) {
      QSYM_ASSERT(instrument_ri != NULL);

      ADDRINT imm = INS_OperandImmediate(ins, OP_1);
      INS_InsertCall(ins,
          IPOINT_BEFORE,
          instrument_ri,
          IARG_FAST_ANALYSIS_CALL,
          IARG_CPU_CONTEXT,
          IARG_REG(dst),
          IARG_IMM(imm),
          IARG_END);
    }
    else if (INS_OperandIsMemory(ins, OP_1)) {
      QSYM_ASSERT(instrument_rm != NULL);

      INS_InsertCall(ins,
          IPOINT_BEFORE,
          instrument_rm,
          IARG_FAST_ANALYSIS_CALL,
          IARG_CPU_CONTEXT,
          IARG_REG(dst),
          IARG_MEM_READ(ins),
          IARG_END);
    }
    else
      UNREACHABLE();
  }
  else if (INS_OperandIsMemory(ins, OP_0)) {
    IARG_TYPE addr_ty, size_ty;
    getMemoryType(write, addr_ty, size_ty);

    if (INS_OperandIsReg(ins, OP_1)) {
      QSYM_ASSERT(instrument_mr != NULL);

      REG src = GET_REG(ins, OP_1);
      INS_InsertCall(ins,
          IPOINT_BEFORE,
          instrument_mr,
          IARG_FAST_ANALYSIS_CALL,
          IARG_CPU_CONTEXT,
          IARG_MEM(ins, addr_ty, size_ty),
          IARG_REG(src),
          IARG_END);
    }
    else if (INS_OperandIsImmediate(ins, OP_1)) {
      QSYM_ASSERT(instrument_mi != NULL);

      ADDRINT imm = INS_OperandImmediate(ins, OP_1);
      INS_InsertCall(ins,
          IPOINT_BEFORE,
          instrument_mi,
          IARG_FAST_ANALYSIS_CALL,
          IARG_CPU_CONTEXT,
          IARG_MEM(ins, addr_ty, size_ty),
          IARG_IMM(imm),
          IARG_END);
    }
    else
      UNREACHABLE();
  }
  else
    UNREACHABLE();
}

static void
analyzeBinary(
    INS ins,
    bool write,
    UINT arg,
    AFUNPTR instrument_rr,
    AFUNPTR instrument_ri,
    AFUNPTR instrument_rm,
    AFUNPTR instrument_mr,
    AFUNPTR instrument_mi) {
  if (INS_OperandIsReg(ins, OP_0)) {
    REG dst = GET_REG(ins, OP_0);
    if (INS_OperandIsReg(ins, OP_1)) {
      assert(instrument_rr != NULL);
      REG src = GET_REG(ins, OP_1);
      INS_InsertCall(ins,
          IPOINT_BEFORE,
          instrument_rr,
          IARG_FAST_ANALYSIS_CALL,
          IARG_CPU_CONTEXT,
          IARG_REG(dst),
          IARG_REG(src),
          IARG_ARG(arg),
          IARG_END);
    }
    else if (INS_OperandIsImmediate(ins, OP_1)) {
      assert(instrument_ri != NULL);
      ADDRINT imm = INS_OperandImmediate(ins, OP_1);

      INS_InsertCall(ins,
          IPOINT_BEFORE,
          instrument_ri,
          IARG_FAST_ANALYSIS_CALL,
          IARG_CPU_CONTEXT,
          IARG_REG(dst),
          IARG_IMM(imm),
          IARG_ARG(arg),
          IARG_END);
    }
    else if (INS_OperandIsMemory(ins, OP_1)) {
      assert(instrument_rm != NULL);
      INS_InsertCall(ins,
          IPOINT_BEFORE,
          instrument_rm,
          IARG_FAST_ANALYSIS_CALL,
          IARG_CPU_CONTEXT,
          IARG_REG(dst),
          IARG_MEM_READ(ins),
          IARG_ARG(arg),
          IARG_END);
    }
    else
      UNREACHABLE();
  }
  else if (INS_OperandIsMemory(ins, OP_0)) {
    IARG_TYPE addr_ty, size_ty;
    getMemoryType(write, addr_ty, size_ty);

    if (INS_OperandIsReg(ins, OP_1)) {
      assert(instrument_mr != NULL);
      REG src = GET_REG(ins, OP_1);
      INS_InsertCall(ins,
          IPOINT_BEFORE,
          instrument_mr,
          IARG_FAST_ANALYSIS_CALL,
          IARG_CPU_CONTEXT,
          IARG_MEM(ins, addr_ty, size_ty),
          IARG_REG(src),
          IARG_ARG(arg),
          IARG_END);
    }
    else if (INS_OperandIsImmediate(ins, OP_1)) {
      assert(instrument_mi != NULL);
      ADDRINT imm = INS_OperandImmediate(ins, OP_1);

      INS_InsertCall(ins,
          IPOINT_BEFORE,
          instrument_mi,
          IARG_FAST_ANALYSIS_CALL,
          IARG_CPU_CONTEXT,
          IARG_MEM(ins, addr_ty, size_ty),
          IARG_IMM(imm),
          IARG_ARG(arg),
          IARG_END);
    }
    else
      UNREACHABLE();
  }
  else
    UNREACHABLE();
}

static void
analyzeBinary(
    INS ins,
    bool write,
    UINT arg1,
    UINT arg2,
    AFUNPTR instrument_rr,
    AFUNPTR instrument_ri,
    AFUNPTR instrument_rm,
    AFUNPTR instrument_mr,
    AFUNPTR instrument_mi) {
  if (INS_OperandIsReg(ins, OP_0)) {
    REG dst = GET_REG(ins, OP_0);
    if (INS_OperandIsReg(ins, OP_1)) {
      assert(instrument_rr != NULL);
      REG src = GET_REG(ins, OP_1);
      INS_InsertCall(ins,
          IPOINT_BEFORE,
          instrument_rr,
          IARG_FAST_ANALYSIS_CALL,
          IARG_CPU_CONTEXT,
          IARG_REG(dst),
          IARG_REG(src),
          IARG_ARG(arg1),
          IARG_ARG(arg2),
          IARG_END);
    }
    else if (INS_OperandIsImmediate(ins, OP_1)) {
      assert(instrument_ri != NULL);
      ADDRINT imm = INS_OperandImmediate(ins, OP_1);

      INS_InsertCall(ins,
          IPOINT_BEFORE,
          instrument_ri,
          IARG_FAST_ANALYSIS_CALL,
          IARG_CPU_CONTEXT,
          IARG_REG(dst),
          IARG_IMM(imm),
          IARG_ARG(arg1),
          IARG_ARG(arg2),
          IARG_END);
    }
    else if (INS_OperandIsMemory(ins, OP_1)) {
      assert(instrument_rm != NULL);
      INS_InsertCall(ins,
          IPOINT_BEFORE,
          instrument_rm,
          IARG_FAST_ANALYSIS_CALL,
          IARG_CPU_CONTEXT,
          IARG_REG(dst),
          IARG_MEM_READ(ins),
          IARG_ARG(arg1),
          IARG_ARG(arg2),
          IARG_END);
    }
    else
      UNREACHABLE();
  }
  else if (INS_OperandIsMemory(ins, OP_0)) {
    IARG_TYPE addr_ty, size_ty;
    getMemoryType(write, addr_ty, size_ty);

    if (INS_OperandIsReg(ins, OP_1)) {
      assert(instrument_mr != NULL);
      REG src = GET_REG(ins, OP_1);
      INS_InsertCall(ins,
          IPOINT_BEFORE,
          instrument_mr,
          IARG_FAST_ANALYSIS_CALL,
          IARG_CPU_CONTEXT,
          IARG_MEM(ins, addr_ty, size_ty),
          IARG_REG(src),
          IARG_ARG(arg1),
          IARG_ARG(arg2),
          IARG_END);
    }
    else if (INS_OperandIsImmediate(ins, OP_1)) {
      assert(instrument_mi != NULL);
      ADDRINT imm = INS_OperandImmediate(ins, OP_1);

      INS_InsertCall(ins,
          IPOINT_BEFORE,
          instrument_mi,
          IARG_FAST_ANALYSIS_CALL,
          IARG_CPU_CONTEXT,
          IARG_MEM(ins, addr_ty, size_ty),
          IARG_IMM(imm),
          IARG_ARG(arg1),
          IARG_ARG(arg2),
          IARG_END);
    }
    else
      UNREACHABLE();
  }
  else
    UNREACHABLE();
}

static void
analyzeTernary(
    INS ins,
    AFUNPTR instrument_rrr,
    AFUNPTR instrument_rrm) {
  QSYM_ASSERT(INS_OperandIsReg(ins, OP_0));
  QSYM_ASSERT(INS_OperandIsReg(ins, OP_1));

  REG dst = INS_OperandReg(ins, OP_0);
  REG src1 = INS_OperandReg(ins, OP_1);

  if (INS_OperandIsReg(ins, OP_2)){
    REG src2 = INS_OperandReg(ins, OP_2);
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        instrument_rrr,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_REG(dst),
        IARG_REG(src1),
        IARG_REG(src2),
        IARG_END);
  }
  else if (INS_OperandIsMemory(ins, OP_2)) {
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        instrument_rrm,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_REG(dst),
        IARG_REG(src1),
        IARG_MEM_READ(ins),
        IARG_END);
  }
  else
    UNREACHABLE();
}

static void
analyzeTernary(
    INS ins,
    UINT arg,
    AFUNPTR instrument_rrr,
    AFUNPTR instrument_rrm) {
  QSYM_ASSERT(INS_OperandIsReg(ins, OP_0));
  QSYM_ASSERT(INS_OperandIsReg(ins, OP_1));

  REG dst = INS_OperandReg(ins, OP_0);
  REG src1 = INS_OperandReg(ins, OP_1);

  if (INS_OperandIsReg(ins, OP_2)){
    REG src2 = INS_OperandReg(ins, OP_2);
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        instrument_rrr,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_REG(dst),
        IARG_REG(src1),
        IARG_REG(src2),
        IARG_ARG(arg),
        IARG_END);
  }
  else if (INS_OperandIsMemory(ins, OP_2)) {
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        instrument_rrm,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_REG(dst),
        IARG_REG(src1),
        IARG_MEM_READ(ins),
        IARG_ARG(arg),
        IARG_END);
  }
  else
    UNREACHABLE();
}

static void
analyzeTernary(
    INS ins,
    UINT arg1,
    UINT arg2,
    AFUNPTR instrument_rrr,
    AFUNPTR instrument_rrm,
    AFUNPTR instrument_rri=NULL) {
  QSYM_ASSERT(INS_OperandIsReg(ins, OP_0));
  QSYM_ASSERT(INS_OperandIsReg(ins, OP_1));

  REG dst = INS_OperandReg(ins, OP_0);
  REG src1 = INS_OperandReg(ins, OP_1);

  if (INS_OperandIsReg(ins, OP_2)){
    REG src2 = INS_OperandReg(ins, OP_2);
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        instrument_rrr,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_REG(dst),
        IARG_REG(src1),
        IARG_REG(src2),
        IARG_ARG(arg1),
        IARG_ARG(arg2),
        IARG_END);
  }
  else if (INS_OperandIsMemory(ins, OP_2)) {
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        instrument_rrm,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_REG(dst),
        IARG_REG(src1),
        IARG_MEM_READ(ins),
        IARG_ARG(arg1),
        IARG_ARG(arg2),
        IARG_END);
  }
  else if (INS_OperandIsImmediate(ins, OP_2)) {
    QSYM_ASSERT(instrument_rri != NULL);
    ADDRINT imm = INS_OperandImmediate(ins, OP_2);
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        instrument_rri,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_REG(dst),
        IARG_REG(src1),
        IARG_IMM(imm),
        IARG_ARG(arg1),
        IARG_ARG(arg2),
        IARG_END);
  }
  else
    UNREACHABLE();
}

static void
analyzeTernaryImm(
    INS ins,
    UINT arg,
    AFUNPTR instrument_rri,
    AFUNPTR instrument_rmi) {
  if (INS_OperandIsReg(ins, OP_0)) {
    REG dst = GET_REG(ins, OP_0);

    if (INS_OperandIsImmediate(ins, OP_2)) {
      ADDRINT imm = INS_OperandImmediate(ins, OP_2);

      if (INS_OperandIsReg(ins, OP_1)) {
        REG src = GET_REG(ins, OP_1);

        INS_InsertCall(ins,
            IPOINT_BEFORE,
            instrument_rri,
            IARG_FAST_ANALYSIS_CALL,
            IARG_CPU_CONTEXT,
            IARG_REG(dst),
            IARG_REG(src),
            IARG_IMM(imm),
            IARG_ARG(arg),
            IARG_END);
      }
      else if (INS_OperandIsMemory(ins, OP_1)) {
        INS_InsertCall(ins,
            IPOINT_BEFORE,
            instrument_rmi,
            IARG_FAST_ANALYSIS_CALL,
            IARG_CPU_CONTEXT,
            IARG_REG(dst),
            IARG_MEM_READ(ins),
            IARG_IMM(imm),
            IARG_ARG(arg),
            IARG_END);
      }
      else
        UNREACHABLE();
    }
    else
      UNREACHABLE();
  }
  else
    UNREACHABLE();
}
} // namespace

void
analyzeBBL(BBL bbl) {
  BBL_InsertCall(bbl,
      IPOINT_BEFORE,
      (AFUNPTR)instrumentBBL,
      IARG_CPU_CONTEXT,
      IARG_END);
}

void
analyzeBinary(INS ins, Kind kind, bool write) {
  // write as the second argument
  analyzeBinary(
      ins, write, kind, write,
      (AFUNPTR)instrumentBinaryRegReg,
      (AFUNPTR)instrumentBinaryRegImm,
      (AFUNPTR)instrumentBinaryRegMem,
      (AFUNPTR)instrumentBinaryMemReg,
      (AFUNPTR)instrumentBinaryMemImm);
}

void
analyzeCarry(INS ins, Kind kind) {
  analyzeBinary(
      ins, true, kind,
      (AFUNPTR)instrumentCarryRegReg,
      (AFUNPTR)instrumentCarryRegImm,
      (AFUNPTR)instrumentCarryRegMem,
      (AFUNPTR)instrumentCarryMemReg,
      (AFUNPTR)instrumentCarryMemImm);
}

void analyzeBs(INS ins, bool inv) {
  analyzeBinary(ins, true, inv,
      (AFUNPTR)instrumentBsRegReg,
      NULL,
      (AFUNPTR)instrumentBsRegMem,
      NULL,
      NULL);
}

void analyzeBswap(INS ins) {
  analyzeUnary(ins, true,
      (AFUNPTR)instrumentBswapReg,
      NULL);
}

void analyzeBt(INS ins, Kind kind) {
  // use kind to indicate btx
  // kind == Invalid -> bt
  // kind == And -> btr
  // kind == Or -> bts
  // kind == Xor -> btc
  bool write = kind == Invalid ? false : true;
  analyzeBinary(
      ins, write, kind,
      (AFUNPTR)instrumentBtRegReg,
      (AFUNPTR)instrumentBtRegImm,
      NULL,
      (AFUNPTR)instrumentBtMemReg,
      (AFUNPTR)instrumentBtMemImm);
}

void analyzeCall(INS ins) {
  // TODO: merge into one
  INS_InsertCall(ins,
      IPOINT_BEFORE,
      (AFUNPTR)instrumentCall,
      IARG_FAST_ANALYSIS_CALL,
      IARG_CPU_CONTEXT,
      IARG_MEMORYWRITE_EA,
      IARG_MEMORYWRITE_SIZE,
      IARG_RETURN_IP,
      IARG_END);

  INS_InsertCall(ins,
      IPOINT_TAKEN_BRANCH,
      (AFUNPTR)instrumentCallAfter,
      IARG_FAST_ANALYSIS_CALL,
      IARG_CPU_CONTEXT,
      IARG_RETURN_IP,
      IARG_END);

  analyzeJmp(ins);
}

void analyzeCbw(INS ins) {
  assert(INS_OperandIsReg(ins, OP_0)
      && INS_OperandIsReg(ins, OP_1));
  REG dst = GET_REG(ins, OP_0);
  REG src = GET_REG(ins, OP_1);

  INS_InsertCall(ins,
      IPOINT_BEFORE,
      (AFUNPTR)instrumentCbw,
      IARG_FAST_ANALYSIS_CALL,
      IARG_CPU_CONTEXT,
      IARG_REG(dst),
      IARG_REG(src),
      IARG_END);
}

void analyzeCwd(INS ins) {
  assert(INS_OperandIsReg(ins, OP_0)
      && INS_OperandIsReg(ins, OP_1));
  REG dst = GET_REG(ins, OP_0);
  REG src = GET_REG(ins, OP_1);

  INS_InsertCall(ins,
      IPOINT_BEFORE,
      (AFUNPTR)instrumentCwd,
      IARG_FAST_ANALYSIS_CALL,
      IARG_CPU_CONTEXT,
      IARG_REG(dst),
      IARG_REG(src),
      IARG_END);
}

void analyzeCmov(INS ins, JccKind jcc_kind, bool inv) {
  if (INS_OperandIsReg(ins, OP_0)) {
    REG dst = GET_REG(ins, OP_0);
    if (INS_OperandIsReg(ins, OP_1)) {
      REG src = GET_REG(ins, OP_1);

      INS_InsertCall(ins,
          IPOINT_BEFORE,
          (AFUNPTR)instrumentCmovRegReg,
          IARG_FAST_ANALYSIS_CALL,
          IARG_CPU_CONTEXT,
          IARG_EXECUTING,
          IARG_REG(dst),
          IARG_REG(src),
          IARG_ARG(jcc_kind),
          IARG_ARG(inv),
          IARG_END);
    }
    else if (INS_OperandIsMemory(ins, OP_1)) {
      INS_InsertCall(ins,
          IPOINT_BEFORE,
          (AFUNPTR)instrumentCmovRegMem,
          IARG_FAST_ANALYSIS_CALL,
          IARG_CPU_CONTEXT,
          IARG_EXECUTING,
          IARG_REG(dst),
          IARG_MEM_READ(ins),
          IARG_ARG(jcc_kind),
          IARG_ARG(inv),
          IARG_END);
    }
    else
      UNREACHABLE();
  }
  else
    UNREACHABLE();
}

void analyzeCmps(INS ins, UINT width) {
    INS_InsertPredicatedCall(ins,
      IPOINT_BEFORE,
      (AFUNPTR)instrumentCmps,
      IARG_FAST_ANALYSIS_CALL,
      IARG_CPU_CONTEXT,
      IARG_MEMORYREAD_EA,
      IARG_MEMORYREAD2_EA,
      IARG_UINT32,
      width,
      IARG_END);
}

void analyzeCmpxchg(INS ins) {
  analyzeBinary(ins, true,
      (AFUNPTR)instrumentCmpxchgRegReg,
      NULL,
      NULL,
      (AFUNPTR)instrumentCmpxchgMemReg,
      NULL);
}

void analyzeCpuid(INS ins) {
  INS_InsertCall(ins,
      IPOINT_AFTER,
      (AFUNPTR)instrumentCpuid,
      IARG_FAST_ANALYSIS_CALL,
      IARG_CPU_CONTEXT,
      IARG_END);
}

void analyzeDiv(INS ins, bool sign) {
  analyzeUnary(ins, false, sign,
      (AFUNPTR)instrumentDivReg,
      (AFUNPTR)instrumentDivMem);
}

void analyzeClear(INS ins) {
  if (INS_OperandIsReg(ins, OP_0)) {
    REG dst = GET_REG(ins, OP_0);
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)instrumentClrReg,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_REG(dst),
        IARG_END);
  }
  else if (INS_OperandIsMemory(ins, OP_0)) {
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)instrumentClrMem,
        IARG_FAST_ANALYSIS_CALL,
        IARG_MEMORYWRITE_EA,
        IARG_MEMORYWRITE_SIZE,
        IARG_END);
  }
  else
    UNREACHABLE();
}

void analyzeIMul(INS ins) {
  if (INS_OperandCount(ins) == 4) {
    if (INS_OperandIsImplicit(ins, OP_1)) {
      if (INS_OperandIsReg(ins, OP_0)) {
        REG src = GET_REG(ins, OP_0);

        INS_InsertCall(ins,
            IPOINT_BEFORE,
            (AFUNPTR)instrumentIMulReg,
            IARG_FAST_ANALYSIS_CALL,
            IARG_CPU_CONTEXT,
            IARG_REG(src),
            IARG_END);
      }
      else if (INS_OperandIsMemory(ins, OP_0)) {
        INS_InsertCall(ins,
            IPOINT_BEFORE,
            (AFUNPTR)instrumentIMulMem,
            IARG_FAST_ANALYSIS_CALL,
            IARG_CPU_CONTEXT,
            IARG_MEM_READ(ins),
            IARG_END);
      }
      else
        UNREACHABLE();
    }
    else {
      if (INS_OperandIsImmediate(ins, OP_2)) {
        if (INS_OperandIsReg(ins, OP_1)) {
          REG dst = GET_REG(ins, OP_0);
          REG src = GET_REG(ins, OP_1);
          ADDRINT imm = INS_OperandImmediate(ins, OP_2);

          INS_InsertCall(ins,
              IPOINT_BEFORE,
              (AFUNPTR)instrumentIMulRegRegImm,
              IARG_FAST_ANALYSIS_CALL,
              IARG_CPU_CONTEXT,
              IARG_REG(dst),
              IARG_REG(src),
              IARG_IMM(imm),
              IARG_END);
        }
        else if (INS_OperandIsMemory(ins, OP_1)) {
          REG dst = GET_REG(ins, OP_0);
          ADDRINT imm = INS_OperandImmediate(ins, OP_2);

          INS_InsertCall(ins,
              IPOINT_BEFORE,
              (AFUNPTR)instrumentIMulRegMemImm,
              IARG_FAST_ANALYSIS_CALL,
              IARG_CPU_CONTEXT,
              IARG_REG(dst),
              IARG_MEM_READ(ins),
              IARG_IMM(imm),
              IARG_END);
        }
        else
          UNREACHABLE();
      }
    }
  }
  else if (INS_OperandCount(ins) == 3) {
    if (INS_OperandIsReg(ins, OP_0)) {
      REG dst = GET_REG(ins, OP_0);
      if (INS_OperandIsReg(ins, OP_1)) {
        REG src = GET_REG(ins, OP_1);

        INS_InsertCall(ins,
            IPOINT_BEFORE,
            (AFUNPTR)instrumentIMulRegReg,
            IARG_FAST_ANALYSIS_CALL,
            IARG_CPU_CONTEXT,
            IARG_REG(dst),
            IARG_REG(src),
            IARG_END);
      }
      else if (INS_OperandIsMemory(ins, OP_1)) {
        INS_InsertCall(ins,
            IPOINT_BEFORE,
            (AFUNPTR)instrumentIMulRegMem,
            IARG_FAST_ANALYSIS_CALL,
            IARG_CPU_CONTEXT,
            IARG_REG(dst),
            IARG_MEM_READ(ins),
            IARG_END);
      }
      else
        UNREACHABLE();
    }
  }
  else
    UNREACHABLE();
}

void analyzeIncDec(INS ins, Kind kind) {
  analyzeUnary(ins, true, kind,
    (AFUNPTR)instrumentIncDecReg,
    (AFUNPTR)instrumentIncDecMem);
}

void analyzeLea(INS ins) {
// LEA r, m
  if (INS_OperandIsReg(ins, OP_0) &&
      INS_OperandIsAddressGenerator(ins, OP_1)) {
    REG src = GET_REG(ins, OP_0);
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)instrumentLeaRegMem,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_REG(src),
        IARG_UINT32,
        INS_MemoryBaseReg(ins),
        IARG_UINT32,
        INS_MemoryIndexReg(ins),
        IARG_ADDRINT,
        INS_MemoryDisplacement(ins),
        IARG_UINT32,
        INS_MemoryScale(ins),
        IARG_END);
  }
  else
    UNREACHABLE();
}

void analyzeLeave(INS ins) {
  INS_InsertCall(ins,
      IPOINT_BEFORE,
      (AFUNPTR)instrumentClrReg,
      IARG_FAST_ANALYSIS_CALL,
      IARG_CPU_CONTEXT,
      IARG_REG(REG_GBP),
      IARG_END);
}

void analyzeMul(INS ins) {
  if (INS_OperandIsReg(ins, OP_0)) {
    REG src = GET_REG(ins, OP_0);
    REG reg_l = getAx(src);
    REG reg_h = getDx(src);
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)instrumentMulReg,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_REG(src),
        IARG_REG(reg_l),
        IARG_REG(reg_h),
        IARG_END);
  }
  else if (INS_OperandIsMemory(ins, OP_0)) {
    USIZE size = INS_MemoryReadSize(ins);
    REG reg_l = getAx(size);
    REG reg_h = getDx(size);

    INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)instrumentMulMem,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_MEM_READ(ins),
        IARG_REG(reg_l),
        IARG_REG(reg_h),
        IARG_END);
  }
}

void analyzeMov(INS ins) {
  analyzeBinary(ins, true,
      (AFUNPTR)instrumentMovRegReg,
      (AFUNPTR)instrumentMovRegImm,
      (AFUNPTR)instrumentMovRegMem,
      (AFUNPTR)instrumentMovMemReg,
      (AFUNPTR)instrumentMovMemImm);
}

void analyzeMovh(INS ins) {
  analyzeBinary(ins, true,
      NULL,
      NULL,
      (AFUNPTR)instrumentMovhRegMem,
      (AFUNPTR)instrumentMovhMemReg,
      NULL);
}

void analyzeMovlz(INS ins, UINT width) {
  analyzeBinary(ins, true, width,
      (AFUNPTR)instrumentMovlzRegReg,
      NULL,
      (AFUNPTR)instrumentMovlzRegMem,
      (AFUNPTR)instrumentMovlzMemReg,
      NULL);
}

void analyzeMovs(INS ins, UINT width) {
  INS_InsertPredicatedCall(ins,
      IPOINT_BEFORE,
      (AFUNPTR)instrumentMovs,
      IARG_FAST_ANALYSIS_CALL,
      IARG_CPU_CONTEXT,
      IARG_MEMORYREAD_EA,
      IARG_MEMORYWRITE_EA,
      IARG_UINT32,
      width,
      IARG_END);
}

void analyzeMovsx(INS ins) {
  analyzeBinary(ins, true,
      (AFUNPTR)instrumentMovsxRegReg,
      NULL,
      (AFUNPTR)instrumentMovsxRegMem,
      NULL,
      NULL);
}

void analyzeMovzx(INS ins) {
  analyzeBinary(ins, true,
      (AFUNPTR)instrumentMovzxRegReg,
      NULL,
      (AFUNPTR)instrumentMovzxRegMem,
      NULL,
      NULL);
}

void
analyzeNegNot(INS ins, Kind kind) {
  analyzeUnary(ins, true, kind,
    (AFUNPTR)instrumentNegNotReg,
    (AFUNPTR)instrumentNegNotMem);
}

void
analyzeJcc(INS ins, JccKind jcc_kind, bool inv) {
  INS_InsertCall(ins,
      IPOINT_BEFORE,
      (AFUNPTR)instrumentJcc,
      IARG_FAST_ANALYSIS_CALL,
      IARG_CPU_CONTEXT,
      IARG_BRANCH_TAKEN,
      IARG_BRANCH_TARGET_ADDR,
      IARG_ARG(jcc_kind),
      IARG_ARG(inv),
      IARG_END);
}

void analyzeJmp(INS ins) {
  if (INS_OperandIsReg(ins, OP_0)) {
    REG src = GET_REG(ins, OP_0);
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)instrumentJmpReg,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_REG(src),
        IARG_END);
  }
  else if (INS_OperandIsMemory(ins, OP_0)) {
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)instrumentJmpMem,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_MEM_READ(ins),
        IARG_END);
  }
}

void analyzePshufb(INS ins) {
  analyzeBinary(ins, true,
      (AFUNPTR)instrumentPshufbRegReg,
      NULL,
      (AFUNPTR)instrumentPshufbRegMem,
      NULL,
      NULL);
}

void analyzePshuf(INS ins, UINT width) {
  analyzeTernaryImm(ins, width,
      (AFUNPTR)instrumentPshufRegRegImm,
      (AFUNPTR)instrumentPshufRegMemImm);
}

void analyzePshufw(INS ins, bool high) {
  analyzeTernaryImm(ins, high,
      (AFUNPTR)instrumentPshufwRegRegImm,
      (AFUNPTR)instrumentPshufwRegMemImm);
}

void analyzePshift(INS ins, Kind kind, UINT width) {
  analyzeBinary(
      ins, true, kind, width,
      (AFUNPTR)instrumentPshiftRegReg,
      (AFUNPTR)instrumentPshiftRegImm,
      (AFUNPTR)instrumentPshiftRegMem,
      NULL,
      NULL);
}

void analyzePop(INS ins) {
  if (INS_OperandIsReg(ins, OP_0)) {
    REG dst = GET_REG(ins, OP_0);

    INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)instrumentPopReg,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_REG(dst),
        IARG_MEMORYREAD_EA,
        IARG_MEMORYREAD_SIZE,
        IARG_END);
  }
  else if (INS_OperandIsMemory(ins, OP_0)) {
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)instrumentPopMem,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_MEM_WRITE(ins),
        IARG_MEMORYREAD_EA,
        IARG_END);
  }
  else
    UNREACHABLE();
}

void analyzePack(INS ins, UINT width, bool sign) {
  analyzeBinary(ins, true, width, sign,
      (AFUNPTR)instrumentPackRegReg,
      NULL,
      (AFUNPTR)instrumentPackRegMem,
      NULL,
      NULL);
}

void analyzePalignr(INS ins) {
  assert(INS_OperandIsImmediate(ins, OP_2));
  assert(INS_OperandIsReg(ins, OP_0));

  REG dst = GET_REG(ins, OP_0);
  ADDRINT imm = INS_OperandImmediate(ins, OP_2);

  if (INS_OperandIsReg(ins, OP_1)) {
    REG src = GET_REG(ins, OP_1);
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)instrumentPalignrReg,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_REG(dst),
        IARG_REG(src),
        IARG_IMM(imm),
        IARG_END);
  }
  else if(INS_OperandIsMemory(ins, OP_1)) {
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)instrumentPalignrMem,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_REG(dst),
        IARG_MEM_READ(ins),
        IARG_IMM(imm),
        IARG_END);
  }
}

void analyzeParith(INS ins, Kind kind, UINT width) {
  analyzeBinary(
      ins, true, kind, width,
      (AFUNPTR)instrumentParithRegReg,
      NULL,
      (AFUNPTR)instrumentParithRegMem,
      NULL,
      NULL);
}

void analyzePcmp(INS ins, Kind kind, UINT width) {
  analyzeBinary(
      ins, true, kind, width,
      (AFUNPTR)instrumentPcmpRegReg,
      NULL,
      (AFUNPTR)instrumentPcmpRegMem,
      NULL,
      NULL);
}

void analyzePextr(INS ins, UINT width) {
  if (INS_OperandIsReg(ins, OP_0)) {
    QSYM_ASSERT(INS_OperandIsReg(ins, OP_1)
        && INS_OperandIsImmediate(ins, OP_2));

    REG dst = GET_REG(ins, OP_0);
    REG src = GET_REG(ins, OP_1);
    ADDRINT imm = INS_OperandImmediate(ins, OP_2);

    INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)instrumentPextrRegRegImm,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_REG(dst),
        IARG_REG(src),
        IARG_IMM(imm),
        IARG_ARG(width),
        IARG_END);
  }
  else if (INS_OperandIsMemory(ins, OP_0)) {
    REG src = GET_REG(ins, OP_1);
    ADDRINT imm = INS_OperandImmediate(ins, OP_2);

    INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)instrumentPextrMemRegImm,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_MEM_WRITE(ins),
        IARG_REG(src),
        IARG_IMM(imm),
        IARG_ARG(width),
        IARG_END);
  }
  else
    UNREACHABLE();
}

void analyzePcmpxchg(INS ins, Kind kind, UINT width) {
  analyzeBinary(
      ins, false, kind, width,
      (AFUNPTR)instrumentPcmpxchgRegReg,
      NULL,
      (AFUNPTR)instrumentPcmpxchgRegMem,
      NULL,
      NULL);
}

void analyzePmaddwd(INS ins) {
  analyzeBinary(
    ins, true,
    (AFUNPTR)instrumentPmaddwdRegReg,
    NULL,
    (AFUNPTR)instrumentPmaddwdRegMem,
    NULL,
    NULL);
}

void analyzePmul(INS ins, UINT width) {
  analyzeBinary(
    ins, true, width,
    (AFUNPTR)instrumentPmulRegReg,
    NULL,
    (AFUNPTR)instrumentPmulRegMem,
    NULL,
    NULL);
}

void analyzePmulhw(INS ins, bool sign) {
  // pass signedness through width
  analyzeBinary(
    ins, true, sign,
    (AFUNPTR)instrumentPmulhwRegReg,
    NULL,
    (AFUNPTR)instrumentPmulhwRegMem,
    NULL,
    NULL);
}

void analyzePmovmskb(INS ins) {
  assert(INS_OperandIsReg(ins, OP_0)
      && INS_OperandIsReg(ins, OP_1));
  REG dst = GET_REG(ins, OP_0);
  REG src = GET_REG(ins, OP_1);

  INS_InsertCall(ins,
      IPOINT_BEFORE,
      (AFUNPTR)instrumentPmovmskb,
      IARG_FAST_ANALYSIS_CALL,
      IARG_CPU_CONTEXT,
      IARG_REG(dst),
      IARG_REG(src),
      IARG_END);
}

void analyzePunpckh(INS ins, UINT width) {
  analyzeBinary(
      ins, false, width,
      (AFUNPTR)instrumentPunpckhRegReg,
      NULL,
      (AFUNPTR)instrumentPunpckhRegMem,
      NULL,
      NULL);
}

void analyzePunpckl(INS ins, UINT width) {
  analyzeBinary(
      ins, false, width,
      (AFUNPTR)instrumentPunpcklRegReg,
      NULL,
      (AFUNPTR)instrumentPunpcklRegMem,
      NULL,
      NULL);
}

void analyzePush(INS ins) {
  if (INS_OperandIsReg(ins, OP_0)) {
    REG src = GET_REG(ins, OP_0);

    INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)instrumentPushReg,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_MEMORYWRITE_EA,
        IARG_MEMORYWRITE_SIZE,
        IARG_REG(src),
        IARG_END);
  }
  else if (INS_OperandIsImmediate(ins, OP_0)) {
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)instrumentPushImm,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_MEMORYWRITE_EA,
        IARG_MEMORYWRITE_SIZE,
        IARG_END);
  }
  else if (INS_OperandIsMemory(ins, OP_0)) {
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)instrumentPushMem,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_MEMORYWRITE_EA,
        IARG_MEM_READ(ins),
        IARG_END);
  }
  else
    UNREACHABLE();
}

void analyzeRdtsc(INS ins) {
  INS_InsertCall(ins,
      IPOINT_BEFORE,
      (AFUNPTR)instrumentRdtsc,
      IARG_FAST_ANALYSIS_CALL,
      IARG_THREAD_CONTEXT,
      IARG_END);
}

void analyzeRdtscp(INS ins) {
  INS_InsertCall(ins,
      IPOINT_BEFORE,
      (AFUNPTR)instrumentRdtscp,
      IARG_FAST_ANALYSIS_CALL,
      IARG_THREAD_CONTEXT,
      IARG_END);
}

void analyzeShift(INS ins, Kind kind) {
  if (INS_OperandIsReg(ins, OP_0)) {
    REG dst = GET_REG(ins, OP_0);

    if (INS_OperandIsImmediate(ins, OP_1)) {
      // shift r, imm
      ADDRINT imm = INS_OperandImmediate(ins, OP_1);
      INS_InsertCall(ins,
          IPOINT_BEFORE,
          (AFUNPTR)instrumentShiftRegImm,
          IARG_FAST_ANALYSIS_CALL,
          IARG_CPU_CONTEXT,
          IARG_REG(dst),
          IARG_IMM(imm),
          IARG_ARG(kind),
          IARG_END);
    }
    else if (INS_OperandIsReg(ins, OP_1)) {
      // shift r, cl
      QSYM_ASSERT(INS_OperandReg(ins, OP_1) == REG_CL);
      INS_InsertCall(ins,
          IPOINT_BEFORE,
          (AFUNPTR)instrumentShiftRegCl,
          IARG_FAST_ANALYSIS_CALL,
          IARG_CPU_CONTEXT,
          IARG_REG(dst),
          IARG_ARG(kind),
          IARG_END);
    }
    else
      UNREACHABLE();
  }
  else if (INS_OperandIsMemory(ins, OP_0)) {
    if (INS_OperandIsImmediate(ins, OP_1)) {
      // shift m, imm
      ADDRINT imm = INS_OperandImmediate(ins, OP_1);
      INS_InsertCall(ins,
          IPOINT_BEFORE,
          (AFUNPTR)instrumentShiftMemImm,
          IARG_FAST_ANALYSIS_CALL,
          IARG_CPU_CONTEXT,
          IARG_MEM_WRITE(ins),
          IARG_IMM(imm),
          IARG_ARG(kind),
          IARG_END);
    }
    else if (INS_OperandIsReg(ins, OP_1)) {
      QSYM_ASSERT(INS_OperandReg(ins, OP_1) == REG_CL);
        INS_InsertCall(ins,
            IPOINT_BEFORE,
            (AFUNPTR)instrumentShiftMemCl,
            IARG_FAST_ANALYSIS_CALL,
            IARG_CPU_CONTEXT,
            IARG_MEM_WRITE(ins),
            IARG_ARG(kind),
            IARG_END);
    }
    else
      UNREACHABLE();
  }
  else
    UNREACHABLE();
}


void analyzeScas(INS ins, UINT width) {
    INS_InsertPredicatedCall(ins,
    IPOINT_BEFORE,
    (AFUNPTR)instrumentScas,
    IARG_FAST_ANALYSIS_CALL,
    IARG_CPU_CONTEXT,
    IARG_MEMORYREAD_EA,
    IARG_UINT32,
    width,
    IARG_END);
}

void analyzeSet(INS ins, JccKind jcc_kind, bool inv) {
  analyzeUnary(ins, true, jcc_kind, inv,
      (AFUNPTR)instrumentSetReg,
      (AFUNPTR)instrumentSetMem);
}

void analyzeShiftd(INS ins, Kind kind) {
    REG src = GET_REG(ins, OP_1);

    if (INS_OperandIsReg(ins, OP_0)) {
      REG dst = GET_REG(ins, OP_0);

      if (INS_OperandIsImmediate(ins, OP_2)) {
        // shld r, r, imm
        ADDRINT imm = INS_OperandImmediate(ins, OP_2);
        INS_InsertCall(ins,
            IPOINT_BEFORE,
            (AFUNPTR)instrumentShiftdRegRegImm,
            IARG_FAST_ANALYSIS_CALL,
            IARG_CPU_CONTEXT,
            IARG_REG(dst),
            IARG_REG(src),
            IARG_IMM(imm),
            IARG_ARG(kind),
            IARG_END);
      }
      else if (INS_OperandIsReg(ins, OP_2)) {
        // shld r, r, cl
        QSYM_ASSERT(INS_OperandReg(ins, OP_2) == REG_CL);
        INS_InsertCall(ins,
            IPOINT_BEFORE,
            (AFUNPTR)instrumentShiftdRegRegCl,
            IARG_FAST_ANALYSIS_CALL,
            IARG_CPU_CONTEXT,
            IARG_REG(dst),
            IARG_REG(src),
            IARG_ARG(kind),
            IARG_END);
      }
      else
        UNREACHABLE();
    }
    else if (INS_OperandIsMemory(ins, OP_0)) {
      if (INS_OperandIsImmediate(ins, OP_2)) {
        // shld m, r, imm
        ADDRINT imm = INS_OperandImmediate(ins, OP_2);
        INS_InsertCall(ins,
            IPOINT_BEFORE,
            (AFUNPTR)instrumentShiftdMemRegImm,
            IARG_FAST_ANALYSIS_CALL,
            IARG_CPU_CONTEXT,
            IARG_MEM_WRITE(ins),
            IARG_REG(src),
            IARG_IMM(imm),
            IARG_ARG(kind),
            IARG_END);
      }
      else if (INS_OperandIsReg(ins, OP_2)) {
        // shld m, r, cl
        QSYM_ASSERT(INS_OperandReg(ins, OP_2) == REG_CL);
        INS_InsertCall(ins,
            IPOINT_BEFORE,
            (AFUNPTR)instrumentShiftdMemRegCl,
            IARG_FAST_ANALYSIS_CALL,
            IARG_CPU_CONTEXT,
            IARG_MEM_WRITE(ins),
            IARG_REG(src),
            IARG_ARG(kind),
            IARG_END);
      }
      else
        UNREACHABLE();
    }
    else
      UNREACHABLE();
}

void analyzeStos(INS ins) {
  if (INS_OperandIsMemory(ins, OP_0)) {
    if (INS_OperandIsReg(ins, OP_2)) {
      REG src = GET_REG(ins, OP_2);

      INS_InsertCall(ins,
          IPOINT_BEFORE,
          (AFUNPTR)instrumentStos,
          IARG_FAST_ANALYSIS_CALL,
          IARG_CPU_CONTEXT,
          IARG_MEMORYWRITE_EA,
          IARG_REG(src),
          IARG_END);
    }
    else
      UNREACHABLE();
  }
  else
    UNREACHABLE();
}

void analyzeXadd(INS ins) {
  analyzeBinary(ins, true,
      (AFUNPTR)instrumentXaddRegReg,
      NULL,
      NULL,
      (AFUNPTR)instrumentXaddMemReg,
      NULL);
}

void analyzeXchg(INS ins) {
  analyzeBinary(ins, true,
      (AFUNPTR)instrumentXchgRegReg,
      NULL,
      (AFUNPTR)instrumentXchgRegMem,
      (AFUNPTR)instrumentXchgMemReg,
      NULL);
}

void analyzeConcretizeReg(INS ins, INT32 i) {
  REG r = GET_REG(ins, i);
  REG fr = REG_FullRegName(r);
  if (isInterestingReg(fr)) {
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)instrumentConcretizeReg,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_REG(r),
        IARG_END);
  }
  else if (REG_is_flags(r)) {
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)instrumentConcretizeEflags,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_END);
  }
}

void analyzeConcretizeMem(INS ins, INT32 i, INT32& count) {
  IARG_TYPE addr_ty, size_ty;
  getMemoryType(ins, i, count, addr_ty, size_ty);
  INS_InsertCall(ins,
      IPOINT_BEFORE,
      (AFUNPTR)instrumentConcretizeMem,
      IARG_FAST_ANALYSIS_CALL,
      IARG_CONTEXT,
      addr_ty,
      size_ty,
      IARG_END);
}

void analyzeDefault(INS ins) {
  INT32 count = 0;
  for (UINT32 i = 0; i < INS_OperandCount(ins); i++) {
    if (INS_OperandIsReg(ins, i))
      analyzeConcretizeReg(ins, i);
    else if (INS_OperandIsMemory(ins, i))
      analyzeConcretizeMem(ins, i, count);
  }
}

void analyzeTernary(INS ins, Kind kind) {
  analyzeTernary(
      ins, kind,
      (AFUNPTR)instrumentTernaryRegRegReg,
      (AFUNPTR)instrumentTernaryRegRegMem);
}

void analyzeVparith(INS ins, Kind kind, UINT width) {
  analyzeTernary(
      ins, kind, width,
      (AFUNPTR)instrumentVparithRegRegReg,
      (AFUNPTR)instrumentVparithRegRegMem);
}

void analyzeVpcmp(INS ins, Kind kind, UINT width) {
  analyzeTernary(
      ins, kind, width,
      (AFUNPTR)instrumentVpcmpRegRegReg,
      (AFUNPTR)instrumentVpcmpRegRegMem);
}

void analyzeVpmaddwd(INS ins) {
  analyzeTernary(
      ins,
      (AFUNPTR)instrumentVpmaddwdRegRegReg,
      (AFUNPTR)instrumentVpmaddwdRegRegMem);
}


void analyzeVinserti128(INS ins) {
  QSYM_ASSERT(INS_OperandIsReg(ins, OP_0));
  QSYM_ASSERT(INS_OperandIsReg(ins, OP_1));
  QSYM_ASSERT(INS_OperandIsImmediate(ins, OP_3));

  REG dst = GET_REG(ins, OP_0);
  REG src1 = GET_REG(ins, OP_1);
  ADDRINT imm = INS_OperandImmediate(ins, OP_3);

  if (INS_OperandIsReg(ins, OP_2)) {
    REG src2 = GET_REG(ins, OP_2);
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)instrumentVinsert128iRegRegRegImm,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_REG(dst),
        IARG_REG(src1),
        IARG_REG(src2),
        IARG_IMM(imm),
        IARG_END);

  }
  else if (INS_OperandIsMemory(ins, OP_2)) {
    INS_InsertCall(ins,
        IPOINT_BEFORE,
        (AFUNPTR)instrumentVinsert128iRegRegMemImm,
        IARG_FAST_ANALYSIS_CALL,
        IARG_CPU_CONTEXT,
        IARG_REG(dst),
        IARG_REG(src1),
        IARG_MEM_READ(ins),
        IARG_IMM(imm),
        IARG_END);
  }
  else
    UNREACHABLE();
}

void analyzeVmovl(INS ins) {
  if (INS_OperandIsReg(ins, OP_0)) {
    REG dst = GET_REG(ins, OP_0);
    if (INS_OperandIsReg(ins, OP_1)) {
      REG src = GET_REG(ins, OP_1);
      if (INS_OperandIsMemory(ins, OP_2)) {
        INS_InsertCall(ins,
            IPOINT_BEFORE,
            (AFUNPTR)instrumentVmovlRegRegMem,
            IARG_FAST_ANALYSIS_CALL,
            IARG_CPU_CONTEXT,
            IARG_REG(dst),
            IARG_REG(src),
            IARG_MEM_READ(ins),
            IARG_END);
      }
      else
        UNREACHABLE();
    }
    else
      UNREACHABLE();
  }
  else if (INS_OperandIsMemory(ins, OP_0)) {
    if (INS_OperandIsReg(ins, OP_1)) {
      REG src = GET_REG(ins, OP_1);
      INS_InsertCall(ins,
          IPOINT_BEFORE,
          (AFUNPTR)instrumentMovMemReg,
          IARG_FAST_ANALYSIS_CALL,
          IARG_CPU_CONTEXT,
          IARG_MEM_WRITE(ins),
          IARG_REG(src),
          IARG_END);
    }
    else
      UNREACHABLE();
  }
  else
    UNREACHABLE();
}

void analyzeVpcmpxchg(INS ins, Kind kind, UINT width) {
  analyzeTernary(
      ins, kind, width,
      (AFUNPTR)instrumentVpcmpxchgRegRegReg,
      (AFUNPTR)instrumentVpcmpxchgRegRegMem);
}

void analyzeVpmulhw(INS ins, bool sign) {
  analyzeTernary(
      ins, sign,
      (AFUNPTR)instrumentVpmulhwRegRegReg,
      (AFUNPTR)instrumentVpmulhwRegRegMem);
}

void analyzeVpmul(INS ins, UINT width) {
  analyzeTernary(
      ins, width,
      (AFUNPTR)instrumentVpmulRegRegReg,
      (AFUNPTR)instrumentVpmulRegRegMem);
}

void analyzeVpshufb(INS ins) {
  analyzeTernary(
      ins,
      (AFUNPTR)instrumentVpshufbRegRegReg,
      (AFUNPTR)instrumentVpshufbRegRegMem);
}

void analyzeVpshift(INS ins, Kind kind, UINT width) {
  analyzeTernary(
      ins, kind, width,
      (AFUNPTR)instrumentVpshiftRegRegReg,
      (AFUNPTR)instrumentVpshiftRegRegMem,
      (AFUNPTR)instrumentVpshiftRegRegImm);
}

void analyzeVpack(INS ins, UINT width, bool sign) {
  analyzeTernary(
      ins, width, sign,
      (AFUNPTR)instrumentVpackRegRegReg,
      (AFUNPTR)instrumentVpackRegRegMem);
}

void analyzeRet(INS ins) {
  INS_InsertCall(ins,
      IPOINT_BEFORE,
      (AFUNPTR)instrumentRet,
      IARG_FAST_ANALYSIS_CALL,
      IARG_CPU_CONTEXT,
      IARG_BRANCH_TARGET_ADDR,
      IARG_END);
}

} // namespace qsym
