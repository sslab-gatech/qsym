#ifndef QSYM_INSTRUMENT_H_
#define QSYM_INSTRUMENT_H_

#include "pin.H"
#include "thread_context.h"

namespace qsym {

#define MEM_ARG  \
  REG base,         \
  REG index,        \
  ADDRINT addr,     \
  UINT32 size,     \
  ADDRINT disp,     \
  UINT32 scale

void instrumentBBL(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx);

void PIN_FAST_ANALYSIS_CALL
instrumentBinaryRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    Kind kind,
    bool write);

void PIN_FAST_ANALYSIS_CALL
instrumentBinaryRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    ADDRINT imm,
    Kind kind,
    bool write);

void PIN_FAST_ANALYSIS_CALL
instrumentBinaryRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    Kind kind,
    bool write);

void PIN_FAST_ANALYSIS_CALL
instrumentBinaryMemReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG src,
    Kind kind,
    bool write);

void PIN_FAST_ANALYSIS_CALL
instrumentBinaryMemImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    ADDRINT imm,
    Kind kind,
    bool write);

void PIN_FAST_ANALYSIS_CALL
instrumentCarryRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentCarryRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    ADDRINT imm,
    Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentCarryRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentCarryMemReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG src,
    Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentCarryMemImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    ADDRINT imm,
    Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentBsRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    bool inv);

void PIN_FAST_ANALYSIS_CALL
instrumentBsRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    bool inv);

void PIN_FAST_ANALYSIS_CALL
instrumentBswapReg(ThreadContext* thread_ctx,
    const CONTEXT* ctx, REG dst);

void PIN_FAST_ANALYSIS_CALL
instrumentBtRegReg(ThreadContext* thread_ctx,
    const CONTEXT* ctx, REG dst, REG src, Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentBtRegImm(ThreadContext* thread_ctx,
    const CONTEXT* ctx, REG dst, ADDRINT imm, Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentBtMemReg(ThreadContext* thread_ctx,
    const CONTEXT* ctx, MEM_ARG, REG src, Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentBtMemImm(ThreadContext* thread_ctx,
    const CONTEXT* ctx, MEM_ARG, ADDRINT imm, Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentCall(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    ADDRINT addr,
    UINT size);

void PIN_FAST_ANALYSIS_CALL
instrumentCbw(ThreadContext* thread_ctx,
    const CONTEXT* ctx, REG to, REG from);

void PIN_FAST_ANALYSIS_CALL
instrumentCwd(ThreadContext* thread_ctx,
    const CONTEXT* ctx, REG, REG);

void PIN_FAST_ANALYSIS_CALL
instrumentCmovRegReg(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    bool taken,
    REG dst,
    REG src,
    JccKind jcc_kind,
    bool inv);

void PIN_FAST_ANALYSIS_CALL
instrumentCmovRegMem(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    bool taken,
    REG dst,
    MEM_ARG,
    JccKind jcc_kind,
    bool inv);

void PIN_FAST_ANALYSIS_CALL
instrumentCmps(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    ADDRINT dst_addr,
    ADDRINT src_addr,
    UINT size);

void PIN_FAST_ANALYSIS_CALL
instrumentCmpxchgRegReg(ThreadContext* thread_ctx,
    const CONTEXT* ctx, REG, REG);

void PIN_FAST_ANALYSIS_CALL
instrumentCmpxchgMemReg(ThreadContext* thread_ctx,
    const CONTEXT* ctx, MEM_ARG, REG);

void PIN_FAST_ANALYSIS_CALL
instrumentCpuid(ThreadContext* thread_ctx, CONTEXT* ctx);

void PIN_FAST_ANALYSIS_CALL
instrumentIncDecReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentIncDecMem(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentDivReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG src,
    bool sign);

void PIN_FAST_ANALYSIS_CALL
instrumentDivMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    bool sign);

void PIN_FAST_ANALYSIS_CALL
instrumentFild(ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG);

void PIN_FAST_ANALYSIS_CALL
instrumentIMulReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG src);

void PIN_FAST_ANALYSIS_CALL
instrumentIMulMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG);

void PIN_FAST_ANALYSIS_CALL
instrumentIMulRegReg(ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG, REG);

void PIN_FAST_ANALYSIS_CALL
instrumentIMulRegMem(ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG,
    MEM_ARG);

void PIN_FAST_ANALYSIS_CALL
instrumentIMulRegRegImm(ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG, REG, ADDRINT);

void PIN_FAST_ANALYSIS_CALL
instrumentIMulRegMemImm(ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG,
    MEM_ARG,
    ADDRINT);

void PIN_FAST_ANALYSIS_CALL
instrumentLeaRegMem(ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG,
    REG,
    REG,
    ADDRINT disp,
    UINT32 scale);

void PIN_FAST_ANALYSIS_CALL
instrumentMovRegReg(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src);

void PIN_FAST_ANALYSIS_CALL
instrumentMovRegImm(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    ADDRINT imm);

void PIN_FAST_ANALYSIS_CALL
instrumentMovRegMem(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG);

void PIN_FAST_ANALYSIS_CALL
instrumentMovMemImm(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    ADDRINT imm);

void PIN_FAST_ANALYSIS_CALL
instrumentMovMemReg(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG src);

void PIN_FAST_ANALYSIS_CALL
instrumentMovhRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG);

void PIN_FAST_ANALYSIS_CALL
instrumentMovhMemReg(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG src);

void PIN_FAST_ANALYSIS_CALL
instrumentMovlzRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentMovlzRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentMovlzMemReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG src,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentMovs(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    ADDRINT src_addr,
    ADDRINT dst_addr,
    UINT32 width);

void PIN_FAST_ANALYSIS_CALL
instrumentMovsxRegMem(ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG,
    MEM_ARG);

void PIN_FAST_ANALYSIS_CALL
instrumentMovsxRegReg(ThreadContext* thread_ctx,
  const CONTEXT* ctx, REG, REG);

void PIN_FAST_ANALYSIS_CALL
instrumentMovzxRegMem(ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG,
    MEM_ARG);

void PIN_FAST_ANALYSIS_CALL
instrumentMovzxRegReg(ThreadContext* thread_ctx,
  const CONTEXT* ctx, REG, REG);

void PIN_FAST_ANALYSIS_CALL
instrumentMulReg(ThreadContext* thread_ctx,
    const CONTEXT* ctx, REG, REG, REG);

void PIN_FAST_ANALYSIS_CALL
instrumentMulMem(ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG, REG);

void PIN_FAST_ANALYSIS_CALL
instrumentNegNotReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentNegNotMem(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentJcc(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    bool taken,
    ADDRINT target,
    JccKind jcc_kind,
    bool inv);

void PIN_FAST_ANALYSIS_CALL
instrumentJmpReg(ThreadContext* thread_ctx, const CONTEXT* ctx, REG);

void PIN_FAST_ANALYSIS_CALL
instrumentJmpMem(ThreadContext* thread_ctx, const CONTEXT* ctx, MEM_ARG);

void PIN_FAST_ANALYSIS_CALL
instrumentPackRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    UINT width,
    bool sign);

void PIN_FAST_ANALYSIS_CALL
instrumentPackRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    UINT width,
    bool sign);

void PIN_FAST_ANALYSIS_CALL
instrumentPalignrReg(ThreadContext* thread_ctx, const CONTEXT* ctx,
    REG, REG, ADDRINT);

void PIN_FAST_ANALYSIS_CALL
instrumentPalignrMem(ThreadContext* thread_ctx, const CONTEXT* ctx,
    REG, MEM_ARG, ADDRINT);

void PIN_FAST_ANALYSIS_CALL
instrumentPcmpRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    Kind kind,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentPcmpRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    Kind kind,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentPextrRegRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    ADDRINT imm,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentPextrMemRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG src,
    ADDRINT imm,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentPmaddwdRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src);

void PIN_FAST_ANALYSIS_CALL
instrumentPmaddwdRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG);

void PIN_FAST_ANALYSIS_CALL
instrumentPmulRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentPmulRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentPmulhwRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    bool sign);

void PIN_FAST_ANALYSIS_CALL
instrumentPmulhwRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    bool sign);

void PIN_FAST_ANALYSIS_CALL
instrumentPcmpxchgRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    Kind kind,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentPcmpxchgRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    Kind kind,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentPmovmskb(ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst, REG src);

void PIN_FAST_ANALYSIS_CALL
instrumentPshufRegRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    ADDRINT imm,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentPshufRegMemImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    ADDRINT imm,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentPshufbRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src);

void PIN_FAST_ANALYSIS_CALL
instrumentPshufbRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG);

void PIN_FAST_ANALYSIS_CALL
instrumentPshufwRegRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    ADDRINT imm,
    bool sign);

void PIN_FAST_ANALYSIS_CALL
instrumentPshufwRegMemImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    ADDRINT imm,
    bool sign);

void PIN_FAST_ANALYSIS_CALL
instrumentPshiftRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    Kind kind,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentPshiftRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    ADDRINT imm,
    Kind kind,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentPshiftRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    Kind kind,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentParithRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    Kind kind,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentParithRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    Kind kind,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentPopReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    ADDRINT src_addr,
    UINT src_size);

void PIN_FAST_ANALYSIS_CALL
instrumentPopMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    ADDRINT src_addr);

void PIN_FAST_ANALYSIS_CALL
instrumentPunpckhRegReg(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentPunpckhRegMem(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentPunpcklRegReg(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentPunpcklRegMem(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentPushReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    ADDRINT dst_addr,
    UINT dst_size,
    REG src);

void PIN_FAST_ANALYSIS_CALL
instrumentPushImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    ADDRINT dst_addr,
    UINT dst_size);

void PIN_FAST_ANALYSIS_CALL
instrumentPushMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    ADDRINT dst_addr,
    MEM_ARG);

void PIN_FAST_ANALYSIS_CALL
instrumentRdtsc(ThreadContext* thread_ctx);

void PIN_FAST_ANALYSIS_CALL
instrumentRdtscp(ThreadContext* thread_ctx);

void PIN_FAST_ANALYSIS_CALL
instrumentScas(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    ADDRINT addr,
    UINT size);

void PIN_FAST_ANALYSIS_CALL
instrumentSetReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    JccKind jcc_c,
    bool inv);

void PIN_FAST_ANALYSIS_CALL
instrumentSetMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    JccKind jcc_c,
    bool inv);

void PIN_FAST_ANALYSIS_CALL
instrumentShiftRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    ADDRINT imm,
    Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentShiftRegCl(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentShiftMemImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    ADDRINT imm,
    Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentShiftMemCl(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentShiftdRegRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    ADDRINT imm,
    Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentShiftdRegRegCl(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentShiftdMemRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG src,
    ADDRINT imm,
    Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentShiftdMemRegCl(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG src,
    Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentStos(ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    ADDRINT dst_addr,
    REG src);

void PIN_FAST_ANALYSIS_CALL
instrumentXaddRegReg(ThreadContext* thread_ctx, const CONTEXT* ctx,
    REG dst, REG src);

void PIN_FAST_ANALYSIS_CALL
instrumentXaddMemReg(ThreadContext* thread_ctx, const CONTEXT* ctx,
    MEM_ARG, REG src);

void PIN_FAST_ANALYSIS_CALL
instrumentXchgRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT *ctx,
    REG dst,
    REG src);

void PIN_FAST_ANALYSIS_CALL
instrumentXchgRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT *ctx,
    REG dst,
    MEM_ARG);

void PIN_FAST_ANALYSIS_CALL
instrumentXchgMemReg(
    ThreadContext* thread_ctx,
    const CONTEXT *ctx,
    MEM_ARG,
    REG src);

void PIN_FAST_ANALYSIS_CALL
instrumentConcretizeReg(ThreadContext* thread_ctx, const CONTEXT *ctx, REG r);

void PIN_FAST_ANALYSIS_CALL
instrumentConcretizeMem(const CONTEXT* ctx, ADDRINT addr, INT32 size);

void PIN_FAST_ANALYSIS_CALL
instrumentConcretizeEflags(ThreadContext* thread_ctx, const CONTEXT *ctx);

void PIN_FAST_ANALYSIS_CALL
instrumentClrReg(ThreadContext* thread_ctx, const CONTEXT *ctx, REG r);

void PIN_FAST_ANALYSIS_CALL
instrumentClrMem(ADDRINT addr, INT32 size);

void PIN_FAST_ANALYSIS_CALL
instrumentTernaryRegRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2,
    Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentTernaryRegRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    MEM_ARG,
    Kind kind);

void PIN_FAST_ANALYSIS_CALL
instrumentVmovlRegRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    MEM_ARG);

void PIN_FAST_ANALYSIS_CALL
instrumentVparithRegRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2,
    Kind kind,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentVparithRegRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    MEM_ARG,
    Kind kind,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentVpcmpRegRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2,
    Kind kind,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentVpcmpRegRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    MEM_ARG,
    Kind kind,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentVinsert128iRegRegRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2,
    ADDRINT imm);

void PIN_FAST_ANALYSIS_CALL
instrumentVinsert128iRegRegMemImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    MEM_ARG,
    ADDRINT imm);

void PIN_FAST_ANALYSIS_CALL
instrumentVpmaddwdRegRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2);

void PIN_FAST_ANALYSIS_CALL
instrumentVpmaddwdRegRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    MEM_ARG);

void PIN_FAST_ANALYSIS_CALL
instrumentVpcmpxchgRegRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2,
    Kind kind,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentVpcmpxchgRegRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    MEM_ARG,
    Kind kind,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentVpmulhwRegRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2,
    bool sign);

void PIN_FAST_ANALYSIS_CALL
instrumentVpmulhwRegRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    MEM_ARG,
    bool sign);

void PIN_FAST_ANALYSIS_CALL
instrumentVpmulRegRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentVpmulRegRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    MEM_ARG,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentVpshufbRegRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2);

void PIN_FAST_ANALYSIS_CALL
instrumentVpshufbRegRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    MEM_ARG);

void PIN_FAST_ANALYSIS_CALL
instrumentVpshiftRegRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2,
    Kind kind,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentVpshiftRegRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    ADDRINT imm,
    Kind kind,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentVpshiftRegRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    MEM_ARG,
    Kind kind,
    UINT width);

void PIN_FAST_ANALYSIS_CALL
instrumentVpackRegRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2,
    UINT width,
    bool sign);

void PIN_FAST_ANALYSIS_CALL
instrumentVpackRegRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    MEM_ARG,
    UINT width,
    bool sign);

void PIN_FAST_ANALYSIS_CALL
instrumentCallAfter(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    ADDRINT ret_addr);

void PIN_FAST_ANALYSIS_CALL
instrumentRet(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    ADDRINT branch_addr);

} // namespace qsym

#endif // QSYM_INSTRUMENT_H_
