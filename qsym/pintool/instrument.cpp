#include <cassert>
#include "thread_context.h"
#include "instrument.h"
#include "expr.h"
#include "solver.h"
#include "third_party/pin/utils.h"

// bit position
namespace qsym {

void instrumentBBL(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx) {
  g_call_stack_manager.visitBasicBlock(PIN_GetContextReg(ctx, REG_INST_PTR));
}

void concretizeReg(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG r,
    ExprRef e) {
  // concretize symbolic register
  QSYM_ASSERT(e != NULL);
  llvm::APInt val = getRegValue(ctx, r);
  g_solver->addValue(e, val);
  thread_ctx->clearExprFromReg(r);
}

void concretizeReg(
    ThreadContext* thread_ctx,
    const CONTEXT *ctx,
    REG r) {
  // concretize if register is symbolic
  ExprRef e = thread_ctx->getExprFromReg(ctx, r);
  if (e == NULL)
    return;

  ADDRINT pc = PIN_GetContextReg(ctx, REG_INST_PTR);
  LOG_INFO("Silent concretization at "
      + hexstr(pc)
      + ": Instruction=" + disassemble(pc)
      + ", REG=" + REG_StringShort(r)
      + ", Expr=" + e->toString() + "\n");

  concretizeReg(thread_ctx, ctx, r, e);
}

void concretizeReg(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG r,
    ExprRef *ptr) {
  // concretize symbolic register
  // and return constant expression
  ExprRef e = *ptr;
  QSYM_ASSERT(e != NULL);
  concretizeReg(thread_ctx, ctx, r, e);
  *ptr = getRegValueExpr(ctx, r);
}

void concretizeMem(ADDRINT addr, UINT32 size, ExprRef e) {
  llvm::APInt value = getMemValue(addr, size);
  g_solver->addValue(e, value);
  g_memory.clearExprFromMem(addr, size);
}

void concretizeMem(
    const CONTEXT* ctx,
    ADDRINT addr,
    UINT32 size) {
  ExprRef e = g_memory.getExprFromMem(addr, size);
  if (e == NULL)
    return;

  ADDRINT pc = PIN_GetContextReg(ctx, REG_INST_PTR);
  LOG_INFO("Silent concretization at "
      + hexstr(pc)
      + ": Instruction=" + disassemble(pc)
      + ", Memory=" + hexstr(addr)
      + ", Size=" + decstr(size)
      + ", Expr=" + e->toString() + "\n");

  concretizeMem(addr, size, e);
}

void concretizeMem(
    ADDRINT addr,
    UINT32 size,
    ExprRef *ptr) {
  ExprRef e = *ptr;
  QSYM_ASSERT(e != NULL);
  concretizeMem(addr, size, e);
  *ptr = getMemValueExpr(addr, size);
}

inline void makeMemConcrete(
    ExprRef *ptr,
    ADDRINT addr,
    UINT32 size) {
  assert(*ptr != NULL);
  llvm::APInt val = getMemValue(addr, size);
  g_solver->addValue(*ptr, val);
  g_memory.clearExprFromMem(addr, size);

  *ptr = NULL;
}

inline void makeExprConcrete(ExprRef e, ADDRINT val) {
  g_solver->addValue(e, val);
}

inline void makeAddrConcrete(ExprRef e, ADDRINT addr) {
  g_solver->addAddr(e, addr);
}

void
fixRegExpr(
    ExprRef* ptr,
    const CONTEXT *ctx,
    REG r) {
  // if NULL, then use constant expr
  if (*ptr == NULL)
    *ptr = getRegValueExpr(ctx, r);
}

void
fixMemExpr(
    ExprRef* ptr,
    ADDRINT addr,
    INT32 size) {
  // if NULL, then use constant expr
  if (*ptr == NULL)
    *ptr = getMemValueExpr(addr, size);
}

void
fixCfExpr(
    ExprRef* ptr,
    const CONTEXT *ctx,
    UINT32 bits) {
  if (*ptr == NULL) {
    ADDRINT cf = getRegValue(ctx, REG_EFLAGS)[EFLAGS_CF];
    *ptr = g_expr_builder->createConstant(cf, bits);
  }
}

void
fixImmExpr(
    ExprRef* ptr,
    ADDRINT imm,
    UINT32 bits) {
  if (*ptr == NULL)
    *ptr = g_expr_builder->createConstant(imm, bits);
}

inline void makeAddrConcrete(ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG) {
  ExprRef e = thread_ctx->getAddrExpr(ctx, base, index, disp, scale);
  if (e) {
    LOG_DEBUG("makeAddrConcrete: pc=" + hexstr(PIN_GetContextReg(ctx, REG_INST_PTR)) + "\n");
    makeAddrConcrete(e, addr);
  }
}

inline ExprRef getExprFromMemSrc(ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG) {
  // TODO: need to change this unintuitive name
  makeAddrConcrete(thread_ctx, ctx, base, index, addr, size, disp, scale);
  return g_memory.getExprFromMem(addr, size);
}

ExprRef extendToDouble(ExprRef e, bool sign) {
  if (sign)
    return g_expr_builder->createSExt(e, e->bits() * 2);
  else
    return g_expr_builder->createZExt(e, e->bits() * 2);
}

static bool
getExprFromRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    ExprRef* expr_dst,
    ExprRef* expr_src) {
  // return false if all expressions are NULL
  *expr_dst = thread_ctx->getExprFromReg(ctx, dst);
  *expr_src = thread_ctx->getExprFromReg(ctx, src);

  if (*expr_dst == NULL
      && *expr_src == NULL)
    return false;

  fixRegExpr(expr_dst, ctx, dst);
  fixRegExpr(expr_src, ctx, src);
  return true;
}

static bool
getExprFromRegImm(
   ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    ADDRINT imm,
    ExprRef* expr_dst,
    ExprRef* expr_src) {
  *expr_dst = thread_ctx->getExprFromReg(ctx, dst);

  if (*expr_dst == NULL)
    return false;

  fixImmExpr(expr_src, imm, (*expr_dst)->bits());
  return true;
}

static bool
getExprFromRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    ExprRef* expr_dst,
    ExprRef* expr_src) {
  *expr_dst = thread_ctx->getExprFromReg(ctx, dst);
  *expr_src = getExprFromMemSrc(thread_ctx, ctx, base, index, addr, size, disp, scale);

  if (*expr_dst == NULL
      && *expr_src == NULL)
    return false;

  fixRegExpr(expr_dst, ctx, dst);
  fixMemExpr(expr_src, addr, size);
  return true;
}

static bool
getExprFromRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    ADDRINT addr,
    UINT size,
    ExprRef* expr_dst,
    ExprRef* expr_src) {
  *expr_dst = thread_ctx->getExprFromReg(ctx, dst);
  *expr_src = g_memory.getExprFromMem(addr, size);

  if (*expr_dst == NULL
      && *expr_src == NULL)
    return false;

  fixRegExpr(expr_dst, ctx, dst);
  fixMemExpr(expr_src, addr, size);
  return true;
}


static bool
getExprFromMemReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG src,
    ExprRef* expr_dst,
    ExprRef* expr_src) {
  makeAddrConcrete(thread_ctx, ctx, base, index, addr, size, disp, scale);
  *expr_dst = g_memory.getExprFromMem(addr, size);
  *expr_src = thread_ctx->getExprFromReg(ctx, src);

  if (*expr_dst == NULL
      && *expr_src == NULL)
    return false;

  fixMemExpr(expr_dst, addr, size);
  fixRegExpr(expr_src, ctx, src);
  return true;
}

static bool
getExprFromMemImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    ADDRINT imm,
    ExprRef* expr_dst,
    ExprRef* expr_src) {
  makeAddrConcrete(thread_ctx, ctx, base, index, addr, size, disp, scale);
  *expr_dst = g_memory.getExprFromMem(addr, size);

  if (*expr_dst == NULL)
    return false;

  fixMemExpr(expr_dst, addr, size);
  fixImmExpr(expr_src, imm, (*expr_dst)->bits());
  return true;
}

static bool
getExprFromMemMem(
  ThreadContext* thread_ctx,
  const CONTEXT* ctx,
  ADDRINT dst_addr,
  ADDRINT src_addr,
  UINT32 size,
  ExprRef* expr_dst,
  ExprRef* expr_src) {
  *expr_dst = g_memory.getExprFromMem(dst_addr, size);
  *expr_src = g_memory.getExprFromMem(src_addr, size);

  if (*expr_dst == NULL
      && *expr_src == NULL)
    return false;

  fixMemExpr(expr_dst, dst_addr, size);
  fixMemExpr(expr_src, src_addr, size);
  return true;
}

static bool
getExprFromRegRegCf(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    ExprRef* expr_dst,
    ExprRef* expr_src,
    ExprRef* expr_cf) {
  UINT32 bits = REG_Size(dst) * CHAR_BIT;
  *expr_cf = thread_ctx->computeCfAsExpr(ctx, bits);

  if (!getExprFromRegReg(thread_ctx, ctx, dst, src, expr_dst, expr_src)
      && *expr_cf == NULL)
    return false;

  fixRegExpr(expr_dst, ctx, dst);
  fixRegExpr(expr_src, ctx, src);
  fixCfExpr(expr_cf, ctx, bits);
  return true;
}

static bool
getExprFromRegImmCf(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    ADDRINT imm,
    ExprRef* expr_dst,
    ExprRef* expr_src,
    ExprRef* expr_cf) {
  UINT32 bits = REG_Size(dst) * CHAR_BIT;
  *expr_cf = thread_ctx->computeCfAsExpr(ctx, bits);

  if (!getExprFromRegImm(thread_ctx, ctx, dst, imm, expr_dst, expr_src)
      && *expr_cf == NULL)
    return false;

  fixRegExpr(expr_dst, ctx, dst);
  fixImmExpr(expr_src, imm, bits);
  fixCfExpr(expr_cf, ctx, bits);
  return true;
}

static bool
getExprFromRegMemCf(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    ExprRef* expr_dst,
    ExprRef* expr_src,
    ExprRef* expr_cf) {
  UINT32 bits = size * CHAR_BIT;
  *expr_cf = thread_ctx->computeCfAsExpr(ctx, bits);

  if (!getExprFromRegMem(
        thread_ctx, ctx, dst,
        base, index, addr, size, disp, scale,
        expr_dst, expr_src)
      && *expr_cf == NULL)
    return false;

  fixRegExpr(expr_dst, ctx, dst);
  fixMemExpr(expr_src, addr, size);
  fixCfExpr(expr_cf, ctx, bits);
  return true;
}

static bool
getExprFromMemRegCf(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG src,
    ExprRef* expr_dst,
    ExprRef* expr_src,
    ExprRef* expr_cf) {
  UINT32 bits = size * CHAR_BIT;
  *expr_cf = thread_ctx->computeCfAsExpr(ctx, bits);

  if (!getExprFromMemReg(
        thread_ctx, ctx,
        base, index, addr, size, disp, scale,
        src, expr_dst, expr_src)
      && *expr_cf == NULL)
    return false;

  fixMemExpr(expr_dst, addr, size);
  fixRegExpr(expr_src, ctx, src);
  fixCfExpr(expr_cf, ctx, bits);
  return true;
}

static bool
getExprFromMemImmCf(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    ADDRINT imm,
    ExprRef* expr_dst,
    ExprRef* expr_src,
    ExprRef* expr_cf) {
  UINT32 bits = size * CHAR_BIT;
  *expr_cf = thread_ctx->computeCfAsExpr(ctx, bits);

  if (!getExprFromMemImm(
        thread_ctx, ctx,
        base, index, addr, size, disp, scale,
        imm, expr_dst, expr_src)
      && *expr_cf == NULL)
    return false;

  fixMemExpr(expr_dst, addr, size);
  fixImmExpr(expr_src, imm, bits);
  fixCfExpr(expr_cf, ctx, bits);
  return true;
}

static bool
getExprFromRegRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2,
    ExprRef* expr_dst,
    ExprRef* expr_src1,
    ExprRef* expr_src2) {
  *expr_src2 = thread_ctx->getExprFromReg(ctx, src2);

  if (!getExprFromRegReg(thread_ctx, ctx, dst, src1, expr_dst, expr_src1)
      && *expr_src2 == NULL)
    return false;

  fixRegExpr(expr_dst, ctx, dst);
  fixRegExpr(expr_src1, ctx, src1);
  fixRegExpr(expr_src2, ctx, src2);
  return true;
}

static bool
getExprFromRegRegImm(
    ThreadContext * thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    ADDRINT imm,
    ExprRef* expr_dst,
    ExprRef* expr_src1,
    ExprRef* expr_src2) {
  UINT32 bits = REG_Size(dst) * CHAR_BIT;

  if (!getExprFromRegReg(thread_ctx, ctx, dst, src1, expr_dst, expr_src1))
    return false;

  // to trigger fixing
  *expr_src2 = NULL;
  fixRegExpr(expr_dst, ctx, dst);
  fixRegExpr(expr_src1, ctx, src1);
  fixImmExpr(expr_src2, imm, bits);
  return true;
}

static bool
getExprFromMemRegImm(
    ThreadContext * thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG src1,
    ADDRINT imm,
    ExprRef* expr_dst,
    ExprRef* expr_src1,
    ExprRef* expr_src2) {
  UINT32 bits = size * CHAR_BIT;

  if (!getExprFromMemReg(
        thread_ctx, ctx,
        base, index, addr, size, disp, scale,
        src1, expr_dst, expr_src1))
    return false;

  *expr_src2 = NULL;
  fixMemExpr(expr_dst, addr, size);
  fixRegExpr(expr_src1, ctx, src1);
  fixImmExpr(expr_src2, imm, bits);
  return true;
}

static bool
getExprFromMemRegReg(
    ThreadContext * thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG src1,
    REG src2,
    ExprRef* expr_dst,
    ExprRef* expr_src1,
    ExprRef* expr_src2) {
  *expr_src2 = thread_ctx->getExprFromReg(ctx, src2);

  if (!getExprFromMemReg(
        thread_ctx, ctx,
        base, index, addr, size, disp, scale,
        src1, expr_dst, expr_src1)
      && *expr_src2 == NULL)
    return false;

  fixMemExpr(expr_dst, addr, size);
  fixRegExpr(expr_src1, ctx, src1);
  fixRegExpr(expr_src2, ctx, src2);
  return true;
}

static void
doMul(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG reg_l,
    REG reg_h,
    ExprRef expr_src,
    ExprRef expr_reg_l,
    UINT32 bits,
    bool sign) {
  expr_src = extendToDouble(expr_src, sign);
  expr_reg_l = extendToDouble(expr_reg_l, sign);

  ExprRef expr_res = g_expr_builder->createBinaryExpr(Mul, expr_src, expr_reg_l);
  thread_ctx->setEflags(CC_OP_UMUL, expr_res, expr_reg_l, expr_src);

  ExprRef expr_res_l = g_expr_builder->createExtract(expr_res, 0, bits);
  thread_ctx->setExprToReg(reg_l, expr_res_l);
  if (reg_h != REG_INVALID()) {
    ExprRef expr_res_h = g_expr_builder->createExtract(expr_res, bits, bits);
    thread_ctx->setExprToReg(reg_h, expr_res_h);
  }
}

static OpKind
getOpKindShift(Kind kind) {
  if (kind == Rol)
    return CC_OP_ROL;
  else if (kind == Ror)
    return CC_OP_ROR;
  else if (kind == AShr)
    return CC_OP_SHR;
  else if (kind == LShr)
    return CC_OP_SHR;
  else if (kind == Shl)
    return CC_OP_SHL;
  else {
    UNREACHABLE();
    return CC_OP_LAST;
  }
}

static ExprRef
normalizeShiftCount(
    ExprRef expr_count,
    UINT32 bits,
    UINT32 modulo) {
  // make counter to be bits-size & modulo
  ExprRef expr_modulo = g_expr_builder->createConstant(modulo, bits);
  expr_count = g_expr_builder->createZExt(expr_count, bits);
  return g_expr_builder->createBinaryExpr(URem, expr_count, expr_modulo);
}

static ExprRef
doShift(
    ThreadContext* thread_ctx,
    Kind kind,
    OpKind op_kind,
    ExprRef expr_dst,
    ExprRef expr_src,
    UINT32 bits) {
  ExprRef expr_res = NULL;
  expr_src = normalizeShiftCount(expr_src, bits, bits);

  if (kind != Ror && kind != Rol) {
    // shift
    expr_res = g_expr_builder->createBinaryExpr(kind, expr_dst, expr_src);
  }
  else {
    // rotate = (x << lshift) | (x >> (bits - lishft))
    ExprRef expr_rshift = expr_src;
    ExprRef expr_lshift = g_expr_builder->createConstant(bits, bits);
    expr_lshift = g_expr_builder->createBinaryExpr(Sub, expr_lshift, expr_rshift);

    if (kind == Rol) {
      // swap if Rol
      ExprRef tmp = expr_lshift;
      expr_lshift = expr_rshift;
      expr_rshift = tmp;
    }

    expr_rshift = g_expr_builder->createBinaryExpr(LShr, expr_dst, expr_rshift);
    expr_lshift = g_expr_builder->createBinaryExpr(Shl, expr_dst, expr_lshift);
    expr_res = g_expr_builder->createBinaryExpr(Or, expr_rshift, expr_lshift);
  }

  thread_ctx->setEflags(op_kind, expr_res, expr_dst, expr_src);
  return expr_res;
}

void PIN_FAST_ANALYSIS_CALL
instrumentShiftRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    ADDRINT imm,
    Kind kind) {
  OpKind op_kind = getOpKindShift(kind);
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;
  UINT bits = REG_Size(dst) * CHAR_BIT;

  if (!getExprFromRegImm(
        thread_ctx,
        ctx,
        dst,
        imm,
        &expr_dst,
        &expr_src)) {
    thread_ctx->invalidateEflags(op_kind);
    return;
  }

  ExprRef expr_res = doShift(thread_ctx, kind, op_kind, expr_dst, expr_src, bits);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentShiftRegCl(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    Kind kind) {
  OpKind op_kind = getOpKindShift(kind);
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;
  UINT bits = REG_Size(dst) * CHAR_BIT;

  if (!getExprFromRegReg(
        thread_ctx,
        ctx,
        dst,
        REG_CL,
        &expr_dst,
        &expr_src)) {
    thread_ctx->invalidateEflags(op_kind);
    return;
  }

  ExprRef expr_res = doShift(thread_ctx, kind, op_kind, expr_dst, expr_src, bits);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentShiftMemImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    ADDRINT imm,
    Kind kind) {
  OpKind op_kind = getOpKindShift(kind);
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;
  UINT bits = size * CHAR_BIT;

  if (!getExprFromMemImm(
        thread_ctx,
        ctx,
        base, index, addr, size, disp, scale,
        imm,
        &expr_dst,
        &expr_src)) {
    thread_ctx->invalidateEflags(op_kind);
    return;
  }

  ExprRef expr_res = doShift(thread_ctx, kind, op_kind, expr_dst, expr_src, bits);
  g_memory.setExprToMem(addr, size, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentShiftMemCl(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    Kind kind) {
  OpKind op_kind = getOpKindShift(kind);
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;
  UINT bits = size * CHAR_BIT;

  if (!getExprFromMemReg(
        thread_ctx,
        ctx,
        base, index, addr, size, disp, scale,
        REG_CL,
        &expr_dst,
        &expr_src)) {
    thread_ctx->invalidateEflags(op_kind);
    return;
  }

  ExprRef expr_res = doShift(thread_ctx, kind, op_kind, expr_dst, expr_src, bits);
  g_memory.setExprToMem(addr, size, expr_res);
}

static ExprRef
doShiftd(
    ThreadContext* thread_ctx,
    Kind kind,
    OpKind op_kind,
    ExprRef expr_dst,
    ExprRef expr_src,
    ExprRef expr_count,
    UINT32 bits) {
  ExprRef expr_res = NULL;
  // modulo by 32
  expr_count = normalizeShiftCount(expr_count, bits * 2, 32);
  if (kind == Shl)
    expr_res = g_expr_builder->createConcat(expr_dst, expr_src);
  else
    expr_res = g_expr_builder->createConcat(expr_src, expr_dst);

  expr_res = g_expr_builder->createBinaryExpr(kind, expr_res, expr_count);
  if (kind == Shl)
    expr_res = g_expr_builder->createExtract(expr_res, bits, bits);
  else
    expr_res = g_expr_builder->createExtract(expr_res, 0, bits);

  // TODO: add eflag support
  thread_ctx->invalidateEflags(op_kind);
  return expr_res;
}

void PIN_FAST_ANALYSIS_CALL
instrumentShiftdRegRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    ADDRINT imm,
    Kind kind) {
  OpKind op_kind = getOpKindShift(kind);
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;
  ExprRef expr_imm = NULL;
  UINT32 bits = REG_Size(dst) * CHAR_BIT;

  if (!getExprFromRegRegImm(
      thread_ctx,
      ctx,
      dst,
      src,
      imm,
      &expr_dst,
      &expr_src,
      &expr_imm)) {
      thread_ctx->invalidateEflags(op_kind);
      return;
  }

  ExprRef expr_res = doShiftd(thread_ctx, kind, op_kind, expr_dst, expr_src, expr_imm, bits);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentShiftdRegRegCl(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    Kind kind) {
  OpKind op_kind = getOpKindShift(kind);
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;
  ExprRef expr_imm = NULL;
  UINT32 bits = REG_Size(dst) * CHAR_BIT;

  if (!getExprFromRegRegReg(
      thread_ctx,
      ctx,
      dst,
      src,
      REG_CL,
      &expr_dst,
      &expr_src,
      &expr_imm)) {
      thread_ctx->invalidateEflags(op_kind);
      return;
  }

  ExprRef expr_res = doShiftd(thread_ctx, kind, op_kind, expr_dst, expr_src, expr_imm, bits);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentShiftdMemRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG src,
    ADDRINT imm,
    Kind kind) {
  OpKind op_kind = getOpKindShift(kind);
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;
  ExprRef expr_imm = NULL;
  UINT32 bits = size * CHAR_BIT;

  if (!getExprFromMemRegImm(
      thread_ctx,
      ctx,
      base, index, addr, size, disp, scale,
      src,
      imm,
      &expr_dst,
      &expr_src,
      &expr_imm)) {
      thread_ctx->invalidateEflags(op_kind);
      return;
  }

  ExprRef expr_res = doShiftd(thread_ctx, kind, op_kind, expr_dst, expr_src, expr_imm, bits);
  g_memory.setExprToMem(addr, size, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentShiftdMemRegCl(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG src,
    Kind kind) {
  OpKind op_kind = getOpKindShift(kind);
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;
  ExprRef expr_imm = NULL;
  UINT32 bits = size * CHAR_BIT;

  if (!getExprFromMemRegReg(
      thread_ctx,
      ctx,
      base, index, addr, size, disp, scale,
      src,
      REG_CL,
      &expr_dst,
      &expr_src,
      &expr_imm)) {
      thread_ctx->invalidateEflags(op_kind);
      return;
  }

  ExprRef expr_res = doShiftd(thread_ctx, kind, op_kind, expr_dst, expr_src, expr_imm, bits);
  g_memory.setExprToMem(addr, size, expr_res);
}

static OpKind
getOpKindBinary(Kind kind) {
  if (kind == Add)
    return CC_OP_ADD;
  else if (kind == Sub)
    return CC_OP_SUB;
  else if (kind == And
      || kind == Or
      || kind == Xor)
    return CC_OP_LOGIC;
  else {
    UNREACHABLE();
    return CC_OP_LAST;
  }
}

static ExprRef
doBinary(
    ThreadContext* thread_ctx,
    Kind kind,
    OpKind op_kind,
    ExprRef expr_dst,
    ExprRef expr_src) {
  ExprRef expr_res = g_expr_builder->createBinaryExpr(kind, expr_dst, expr_src);
  thread_ctx->setEflags(op_kind, expr_res, expr_dst, expr_src);
  return expr_res;
}

void PIN_FAST_ANALYSIS_CALL
instrumentBinaryRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    Kind kind,
    bool write) {
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;
  OpKind op_kind = getOpKindBinary(kind);

  if (!getExprFromRegReg(
        thread_ctx,
        ctx,
        dst,
        src,
        &expr_dst,
        &expr_src)) {
    thread_ctx->invalidateEflags(op_kind);
    return;
  }

  ExprRef expr_res = doBinary(thread_ctx, kind, op_kind, expr_dst, expr_src);
  if (write)
    thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentBinaryRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    ADDRINT imm,
    Kind kind,
    bool write) {
  OpKind op_kind = getOpKindBinary(kind);
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegImm(
        thread_ctx,
        ctx,
        dst,
        imm,
        &expr_dst,
        &expr_src)) {
    thread_ctx->invalidateEflags(op_kind);
    return;
  }

  ExprRef expr_res = doBinary(thread_ctx, kind, op_kind, expr_dst, expr_src);
  if (write)
    thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentBinaryRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    Kind kind,
    bool write) {
  OpKind op_kind = getOpKindBinary(kind);
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegMem(
        thread_ctx,
        ctx,
        dst,
        base, index, addr, size, disp, scale,
        &expr_dst,
        &expr_src)) {
    thread_ctx->invalidateEflags(op_kind);
    return;
  }

  ExprRef expr_res = doBinary(thread_ctx, kind, op_kind, expr_dst, expr_src);
  if (write)
    thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentBinaryMemReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG src,
    Kind kind,
    bool write) {
  OpKind op_kind = getOpKindBinary(kind);
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromMemReg(
        thread_ctx,
        ctx,
        base, index, addr, size, disp, scale,
        src,
        &expr_dst,
        &expr_src)) {
    thread_ctx->invalidateEflags(op_kind);
    return;
  }

  ExprRef expr_res = doBinary(thread_ctx, kind, op_kind, expr_dst, expr_src);
  if (write)
    g_memory.setExprToMem(addr, size, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentBinaryMemImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    ADDRINT imm,
    Kind kind,
    bool write) {
  OpKind op_kind = getOpKindBinary(kind);
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromMemImm(
      thread_ctx,
      ctx,
      base, index, addr, size, disp, scale,
      imm,
      &expr_dst,
      &expr_src)) {
    thread_ctx->invalidateEflags(op_kind);
    return;
  }

  ExprRef expr_res = doBinary(thread_ctx, kind, op_kind, expr_dst, expr_src);
  if (write)
    g_memory.setExprToMem(addr, size, expr_res);
}

static ExprRef
doCarry(
    ThreadContext* thread_ctx,
    Kind kind,
    OpKind op_kind,
    ExprRef expr_dst,
    ExprRef expr_src,
    ExprRef expr_cf) {
  expr_dst = g_expr_builder->createBinaryExpr(kind, expr_dst, expr_cf);
  ExprRef expr_res = g_expr_builder->createBinaryExpr(kind, expr_dst, expr_src);
  thread_ctx->setEflags(op_kind, expr_res, expr_dst, expr_src);
  return expr_res;
}

void
instrumentCarryRegReg(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    Kind kind) {
  QSYM_ASSERT(kind == Add || kind == Sub);
  OpKind op_kind = getOpKindBinary(kind);
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;
  ExprRef expr_cf = NULL;

  if (!getExprFromRegRegCf(
        thread_ctx,
        ctx,
        dst,
        src,
        &expr_dst,
        &expr_src,
        &expr_cf)) {
    thread_ctx->invalidateEflags(op_kind);
    return;
  }

  ExprRef expr_res = doCarry(thread_ctx, kind, op_kind, expr_dst, expr_src, expr_cf);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentCarryRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    ADDRINT imm,
    Kind kind) {
  QSYM_ASSERT(kind == Add || kind == Sub);
  OpKind op_kind = getOpKindBinary(kind);
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;
  ExprRef expr_cf = NULL;

  if (!getExprFromRegImmCf(
        thread_ctx,
        ctx,
        dst,
        imm,
        &expr_dst,
        &expr_src,
        &expr_cf)) {
    thread_ctx->invalidateEflags(op_kind);
    return;
  }

  ExprRef expr_res = doCarry(thread_ctx, kind, op_kind, expr_dst, expr_src, expr_cf);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentCarryRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    Kind kind) {
  QSYM_ASSERT(kind == Add || kind == Sub);
  OpKind op_kind = getOpKindBinary(kind);
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;
  ExprRef expr_cf = NULL;

  if (!getExprFromRegMemCf(
        thread_ctx,
        ctx,
        dst,
        base, index, addr, size, disp, scale,
        &expr_dst,
        &expr_src,
        &expr_cf)) {
    thread_ctx->invalidateEflags(op_kind);
    return;
  }

  ExprRef expr_res = doCarry(thread_ctx, kind, op_kind, expr_dst, expr_src, expr_cf);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentCarryMemReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG src,
    Kind kind) {
  QSYM_ASSERT(kind == Add || kind == Sub);
  OpKind op_kind = getOpKindBinary(kind);
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;
  ExprRef expr_cf = NULL;

  if (!getExprFromMemRegCf(
        thread_ctx,
        ctx,
        base, index, addr, size, disp, scale,
        src,
        &expr_dst,
        &expr_src,
        &expr_cf)) {
    thread_ctx->invalidateEflags(op_kind);
    return;
  }

  ExprRef expr_res = doCarry(thread_ctx, kind, op_kind, expr_dst, expr_src, expr_cf);
  g_memory.setExprToMem(addr, size, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentCarryMemImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    ADDRINT imm,
    Kind kind) {
  QSYM_ASSERT(kind == Add || kind == Sub);
  OpKind op_kind = getOpKindBinary(kind);
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;
  ExprRef expr_cf = NULL;

  if (!getExprFromMemImmCf(
        thread_ctx,
        ctx,
        base, index, addr, size, disp, scale,
        imm,
        &expr_dst,
        &expr_src,
        &expr_cf)) {
    thread_ctx->invalidateEflags(op_kind);
    return;
  }

  ExprRef expr_res = doCarry(thread_ctx, kind, op_kind, expr_dst, expr_src, expr_cf);
  g_memory.setExprToMem(addr, size, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentCall(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    ADDRINT addr,
    UINT size) {
  // TODO: need to handle symbolic call
  concretizeReg(thread_ctx, ctx, REG_STACK_PTR);
  // Clear by return address
  g_memory.clearExprFromMem(addr, size);
}

void PIN_FAST_ANALYSIS_CALL
instrumentCbw(ThreadContext* thread_ctx,
    const CONTEXT* ctx, REG to, REG from) {
  INT32 bits = REG_Size(to) * CHAR_BIT;
  ExprRef expr_from = thread_ctx->getExprFromReg(ctx, from);
  ExprRef expr_to = NULL;

  if (expr_from == NULL) {
    thread_ctx->clearExprFromReg(to);
    return;
  }

  expr_to = g_expr_builder->createSExt(expr_from, bits);
  thread_ctx->setExprToReg(to, expr_to);
}

void PIN_FAST_ANALYSIS_CALL
instrumentCwd(ThreadContext* thread_ctx,
    const CONTEXT* ctx, REG dx, REG ax) {
  // dst = REG_*DX & src = REG_*AX
  INT32 bits = REG_Size(ax) * CHAR_BIT;
  ExprRef expr_ax = thread_ctx->getExprFromReg(ctx, ax);
  ExprRef expr_dx = NULL;

  if (expr_ax == NULL) {
    thread_ctx->clearExprFromReg(dx);
    return;
  }

  expr_dx = g_expr_builder->createSExt(expr_ax, bits * 2);
  expr_dx = g_expr_builder->createExtract(expr_dx, bits, bits);
  thread_ctx->setExprToReg(dx, expr_dx);
}

static ExprRef
doBsf(ExprRef expr_src) {
  ExprRef expr_bit_one = g_expr_builder->createConstant(1, 1);
  UINT32 bits = expr_src->bits();
  ExprRef expr_res = g_expr_builder->createConstant(bits, bits);

  for (UINT i = expr_src->bits() - 1; i-- > 0;) {
    ExprRef expr_bit = g_expr_builder->createExtract(expr_src, i, 1);
    expr_res = g_expr_builder->createIte(
        g_expr_builder->createEqual(expr_bit, expr_bit_one),
        g_expr_builder->createConstant(i, bits),
        expr_res);
  }

  return expr_res;
}

static ExprRef
doBsr(ExprRef expr_src) {
  ExprRef expr_bit_one = g_expr_builder->createConstant(1, 1);
  UINT32 bits = expr_src->bits();
  ExprRef expr_res = g_expr_builder->createConstant(bits, bits);

  expr_res = g_expr_builder->createConstant(bits, bits);
  for (UINT i = 0; i < expr_src->bits(); i++) {
     ExprRef expr_bit = g_expr_builder->createExtract(expr_src, i, 1);
     expr_res = g_expr_builder->createIte(
         g_expr_builder->createEqual(expr_bit, expr_bit_one),
         g_expr_builder->createConstant(i, bits),
         expr_res);
  }

  return expr_res;
}

static void
doBs(
    ThreadContext *thread_ctx,
    REG dst,
    ExprRef expr_src,
    bool inv) {
  ExprRef expr_res = NULL;

  // bs needs to change ZF, but its behavior is same with sub
  // except for undefined flags. so, reuse sub's logic
  thread_ctx->setEflags(
      CC_OP_SUB,
      expr_src, expr_src,
      g_expr_builder->createConstant(0, expr_src->bits()));

  if (inv)
    expr_res = doBsr(expr_src);
  else
    expr_res = doBsf(expr_src);

  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentBsRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    bool inv) {
  ExprRef expr_src = thread_ctx->getExprFromReg(ctx, src);

  if (expr_src == NULL) {
    thread_ctx->clearExprFromReg(dst);
    thread_ctx->invalidateEflags(CC_OP_SUB);
    return;
  }

  doBs(thread_ctx, dst, expr_src, inv);
}

void PIN_FAST_ANALYSIS_CALL
instrumentBsRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    bool inv) {
  makeAddrConcrete(thread_ctx, ctx, base, index, addr, size, disp, scale);
  ExprRef expr_src = g_memory.getExprFromMem(addr, size);

  if (expr_src == NULL) {
    g_memory.clearExprFromMem(addr, size);
    thread_ctx->invalidateEflags(CC_OP_SUB);
    return;
  }

  doBs(thread_ctx, dst, expr_src, inv);
}

void PIN_FAST_ANALYSIS_CALL
instrumentBswapReg(ThreadContext* thread_ctx, const CONTEXT* ctx,
    REG dst) {
  ExprRef expr_dst = thread_ctx->getExprFromReg(ctx, dst);
  std::list<ExprRef> exprs;

  if (expr_dst) {
    for (UINT32 i = 0; i < expr_dst->bits() / CHAR_BIT; i++)
      exprs.push_back(g_expr_builder->createExtract(expr_dst, i * CHAR_BIT, CHAR_BIT));

    ExprRef expr_res = g_expr_builder->createConcat(exprs);
    thread_ctx->setExprToReg(dst, expr_res);
  }
}

static ExprRef doBt(ThreadContext* thread_ctx, ExprRef expr_dst, ADDRINT imm,
                 UINT bits, Kind kind) {
  ADDRINT mask = 0;
  imm = imm & (bits - 1);

  switch (kind) {
    case Or:
    case Xor:
      mask = 1 << imm;
      break;
    case And:
      mask = ~(1 << imm);
      break;
    case Invalid:
      mask = 0;
      break;
    default:
      UNREACHABLE();
  }

  ExprRef expr_bit = g_expr_builder->createExtract(expr_dst, imm, 1);
  expr_bit = g_expr_builder->bitToBool(expr_bit);
  thread_ctx->setEflags(CC_OP_BT, expr_bit, NULL, NULL);

  if (kind != Invalid) {
    ExprRef expr_mask = g_expr_builder->createConstant(mask, bits);
    expr_dst = g_expr_builder->createBinaryExpr(kind, expr_dst, expr_mask);
    return expr_dst;
  }
  else
    return NULL;
}

void PIN_FAST_ANALYSIS_CALL
instrumentBtRegReg(ThreadContext* thread_ctx, const CONTEXT* ctx,
    REG dst, REG src, Kind kind) {
  UINT bits = REG_Size(dst) * CHAR_BIT;
  ExprRef expr_dst = thread_ctx->getExprFromReg(ctx, dst);
  // Concretize for non-linear operation
  concretizeReg(thread_ctx, ctx, src);

  if (expr_dst == NULL) {
    thread_ctx->invalidateEflags(CC_OP_BT);
    return;
  }

  expr_dst = doBt(thread_ctx, expr_dst, getRegValue(ctx, src).getZExtValue(),
                  bits, kind);

  if (kind != Invalid) {
    assert(expr_dst != NULL);
    thread_ctx->setExprToReg(dst, expr_dst);
  }
}

void PIN_FAST_ANALYSIS_CALL
instrumentBtRegImm(ThreadContext* thread_ctx,
    const CONTEXT* ctx, REG dst, ADDRINT imm, Kind kind) {
  ExprRef expr_dst = thread_ctx->getExprFromReg(ctx, dst);

  if (expr_dst == NULL) {
    thread_ctx->invalidateEflags(CC_OP_BT);
    return;
  }

  expr_dst = doBt(thread_ctx, expr_dst, imm, REG_Size(dst) * CHAR_BIT, kind);

  if (kind != Invalid) {
    assert(expr_dst != NULL);
    thread_ctx->setExprToReg(dst, expr_dst);
  }
}

void PIN_FAST_ANALYSIS_CALL
instrumentBtMemReg(ThreadContext* thread_ctx,
    const CONTEXT* ctx, MEM_ARG, REG src, Kind kind) {
  UINT bits = size * CHAR_BIT;
  ExprRef expr_dst = NULL;
  ADDRINT imm = 0;

  concretizeReg(thread_ctx, ctx, src);
  imm = getRegValue(ctx, src).getZExtValue();
  expr_dst = g_memory.getExprFromMem(addr, size);

  if (expr_dst == NULL) {
    thread_ctx->invalidateEflags(CC_OP_BT);
    return;
  }

  expr_dst = doBt(thread_ctx, expr_dst, imm, bits, kind);

  if (kind != Invalid) {
    assert(expr_dst != NULL);
    g_memory.setExprToMem(addr, size, expr_dst);
  }
}

void PIN_FAST_ANALYSIS_CALL
instrumentBtMemImm(ThreadContext* thread_ctx,
    const CONTEXT* ctx, MEM_ARG, ADDRINT imm, Kind kind) {
  UINT bits = size * CHAR_BIT;
  ExprRef expr_dst = g_memory.getExprFromMem(addr, size);

  if (expr_dst == NULL) {
    thread_ctx->invalidateEflags(CC_OP_BT);
    return;
  }

  expr_dst = doBt(thread_ctx, expr_dst, imm, bits, kind);

  if (kind != Invalid) {
    assert(expr_dst != NULL);
    g_memory.setExprToMem(addr, size, expr_dst);
  }
}

void PIN_FAST_ANALYSIS_CALL
instrumentCmovRegReg(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    bool taken,
    REG dst,
    REG src,
    JccKind jcc_kind,
    bool inv) {
  // call instrumentJcc to trigger solver
  instrumentJcc(thread_ctx, ctx, taken, 0, jcc_kind, inv);
  if (taken)
    instrumentMovRegReg(thread_ctx, ctx, dst, src);
}

void PIN_FAST_ANALYSIS_CALL
instrumentCmovRegMem(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    bool taken,
    REG dst,
    MEM_ARG,
    JccKind jcc_kind,
    bool inv) {
  // call instrumentJcc to trigger solver
  instrumentJcc(thread_ctx, ctx, taken, 0, jcc_kind, inv);
  if (taken)
    instrumentMovRegMem(thread_ctx, ctx, dst,
        base, index, addr, size, disp, scale);
}

void PIN_FAST_ANALYSIS_CALL
instrumentCmps(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    ADDRINT dst_addr,
    ADDRINT src_addr,
    UINT size) {
  // TODO: add symbolic access
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  concretizeReg(thread_ctx, ctx, REG_GSI);
  concretizeReg(thread_ctx, ctx, REG_GDI);

  if (!getExprFromMemMem(
        thread_ctx,
        ctx,
        dst_addr,
        src_addr,
        size,
        &expr_dst,
        &expr_src)) {
    thread_ctx->invalidateEflags(CC_OP_SUB);
    return;
  }

  ExprRef expr_res = g_expr_builder->createBinaryExpr(Sub, expr_dst, expr_src);
  thread_ctx->setEflags(CC_OP_SUB, expr_res, expr_dst, expr_src);
}

void PIN_FAST_ANALYSIS_CALL
instrumentCmpxchgRegReg(ThreadContext* thread_ctx,
    const CONTEXT* ctx, REG dst, REG src) {
  // TODO: cmpxchg can implemented using ITE.
  // however, we need to make ZF symbolic.
  // instead, we try to solve the false case and follow the true case
  REG ax = getAx(src);
  ExprRef expr_dst = thread_ctx->getExprFromReg(ctx, dst);
  ExprRef expr_src = thread_ctx->getExprFromReg(ctx, src);
  ExprRef expr_ax = thread_ctx->getExprFromReg(ctx, ax);

  if (expr_dst == NULL
      && expr_src == NULL
      && expr_ax == NULL) {
    thread_ctx->invalidateEflags(CC_OP_SUB);
    return;
  }

  llvm::APInt val_dst = getRegValue(ctx, dst);
  llvm::APInt val_ax = getRegValue(ctx, ax);

  // mimic cmp
  instrumentBinaryRegReg(
      thread_ctx,
      ctx, dst, ax,
      Sub, false);

  // solve
  instrumentJcc(thread_ctx, ctx, true,
      0,
      JCC_Z,
      val_ax != val_dst);

  if (val_ax == val_dst)
    instrumentMovRegReg(thread_ctx, ctx, dst, src);
  else
    instrumentMovRegReg(thread_ctx, ctx, ax, dst);
}

void PIN_FAST_ANALYSIS_CALL
instrumentCmpxchgMemReg(ThreadContext* thread_ctx,
    const CONTEXT* ctx, MEM_ARG, REG src) {
  REG ax = getAx(src);
  ExprRef expr_dst = g_memory.getExprFromMem(addr, size);
  ExprRef expr_src = thread_ctx->getExprFromReg(ctx, src);
  ExprRef expr_ax = thread_ctx->getExprFromReg(ctx, ax);

  if (expr_dst == NULL
      && expr_src == NULL
      && expr_ax == NULL) {
    thread_ctx->invalidateEflags(CC_OP_SUB);
    return;
  }

  llvm::APInt val_dst = getMemValue(addr, size);
  llvm::APInt val_ax = getRegValue(ctx, ax);

  // mimic cmp
  instrumentBinaryRegMem(
      thread_ctx, ctx, ax,
      base, index, addr, size, disp, scale,
      Sub, false);

  // solve
  instrumentJcc(thread_ctx, ctx, true,
      0,
      JCC_Z,
      val_ax != val_dst);

  if (val_ax == val_dst)
    instrumentMovMemReg(thread_ctx, ctx, base, index, addr, size, disp, scale, src);
  else
    instrumentMovRegMem(thread_ctx, ctx, ax, base, index, addr, size, disp, scale);
}

void PIN_FAST_ANALYSIS_CALL
instrumentCpuid(ThreadContext* thread_ctx, CONTEXT* ctx) {
  thread_ctx->clearExprFromReg(REG_EAX);
  thread_ctx->clearExprFromReg(REG_EBX);
  thread_ctx->clearExprFromReg(REG_ECX);
  thread_ctx->clearExprFromReg(REG_EDX);

  // Set 'FAKE' cpuid to prevent that glibc uses optimized libraries
  PIN_SetContextReg(ctx, REG_GBX, 0x46414b45);
  PIN_ExecuteAt(ctx);
}

static OpKind
getOpKindIncDec(Kind kind) {
  if (kind == Add)
    return CC_OP_INC;
  else if (kind == Sub)
    return CC_OP_DEC;
  else {
    UNREACHABLE();
    return CC_OP_LAST;
  }
}

static ExprRef
doIncDec(ThreadContext* thread_ctx, ExprRef expr_dst, Kind kind, OpKind op_kind) {
  if (expr_dst == NULL) {
    thread_ctx->invalidateEflags(op_kind);
    return NULL;
  }

  ExprRef expr_src = g_expr_builder->createConstant(1, expr_dst->bits());
  ExprRef expr_res = g_expr_builder->createBinaryExpr(kind, expr_dst, expr_src);
  thread_ctx->setEflags(op_kind, expr_res, expr_dst, expr_src);
  return expr_res;
}

void PIN_FAST_ANALYSIS_CALL
instrumentIncDecReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    Kind kind) {
  ExprRef expr_dst = thread_ctx->getExprFromReg(ctx, dst);
  OpKind op_kind = getOpKindIncDec(kind);

  ExprRef expr_res = doIncDec(thread_ctx, expr_dst, kind, op_kind);
  if (expr_res != NULL)
    thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentIncDecMem(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    Kind kind) {
  makeAddrConcrete(thread_ctx, ctx, base, index, addr, size, disp, scale);
  ExprRef expr_dst = g_memory.getExprFromMem(addr, size);
  OpKind op_kind = getOpKindIncDec(kind);

  ExprRef expr_res = doIncDec(thread_ctx, expr_dst, kind, op_kind);
  if (expr_res != NULL)
    g_memory.setExprToMem(addr, size, expr_res);
}

static void
doDiv(
    ThreadContext* thread_ctx,
    REG reg_h,
    REG reg_l,
    ExprRef expr_src,
    ExprRef expr_div,
    bool sign) {
  UINT32 bits = REG_Size(reg_h) * CHAR_BIT;

  expr_src = extendToDouble(expr_src, sign);
  QSYM_ASSERT(expr_src->bits() == expr_div->bits());

  ExprRef expr_q = g_expr_builder->createExtract(
                    g_expr_builder->createBinaryExpr(
                      sign ? SDiv : UDiv,
                      expr_div,
                      expr_src),
                    0, bits);

  ExprRef expr_r = g_expr_builder->createExtract(
                    g_expr_builder->createBinaryExpr(
                      sign ? SRem : URem,
                      expr_div,
                      expr_src),
                    0, bits);

  thread_ctx->setExprToReg(reg_l, expr_q);
  thread_ctx->setExprToReg(reg_h, expr_r);
}

void PIN_FAST_ANALYSIS_CALL
instrumentDivReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG src,
    bool sign) {
  REG reg_l = getAx(src);
  REG reg_h = getDx(src);

  // DIV: eflags are undefined
  thread_ctx->invalidateEflags(CC_OP_UMUL);

  ExprRef expr_src = NULL;
  ExprRef expr_reg_l = NULL;
  ExprRef expr_reg_h = NULL;

  if (!getExprFromRegRegReg(
      thread_ctx,
      ctx,
      src,
      reg_l,
      reg_h,
      &expr_src,
      &expr_reg_l,
      &expr_reg_h))
    return;

  ExprRef expr_div = g_expr_builder->createConcat(expr_reg_h, expr_reg_l);
  doDiv(thread_ctx, reg_h, reg_l, expr_src, expr_div, sign);
}

void
instrumentDivMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    bool sign) {
  REG reg_l = getAx(size);
  REG reg_h = getDx(size);

  // DIV: eflags are undefined
  thread_ctx->invalidateEflags(CC_OP_UMUL);

  ExprRef expr_src = NULL;
  ExprRef expr_reg_l = NULL;
  ExprRef expr_reg_h = NULL;

  if (!getExprFromMemRegReg(
      thread_ctx,
      ctx,
      base, index, addr, size, disp, scale,
      reg_l,
      reg_h,
      &expr_src,
      &expr_reg_l,
      &expr_reg_h))
    return;

  ExprRef expr_div = g_expr_builder->createConcat(expr_reg_h, expr_reg_l);
  doDiv(thread_ctx, reg_h, reg_l, expr_src, expr_div, sign);
}

static void
instrumentMulReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG src,
    REG reg_l,
    REG reg_h,
    bool sign) {
  ExprRef expr_src = NULL;
  ExprRef expr_reg_l = NULL;
  UINT32 bits = REG_Size(src) * CHAR_BIT;

  if (!getExprFromRegReg(
        thread_ctx,
        ctx,
        src,
        reg_l,
        &expr_src,
        &expr_reg_l)) {
    thread_ctx->invalidateEflags(CC_OP_UMUL);
    thread_ctx->clearExprFromReg(reg_l);
    if (reg_h != REG_INVALID())
      thread_ctx->clearExprFromReg(reg_h);
    return;
  }

  doMul(thread_ctx, ctx, reg_l, reg_h, expr_src, expr_reg_l, bits, sign);
}

static void
instrumentMulMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG reg_l,
    REG reg_h,
    bool sign) {
  ExprRef expr_src = NULL;
  ExprRef expr_reg_l = NULL;
  UINT32 bits = size * CHAR_BIT;

  if (!getExprFromMemReg(
        thread_ctx,
        ctx,
        base, index, addr, size, disp, scale,
        reg_l,
        &expr_src,
        &expr_reg_l)) {
    thread_ctx->invalidateEflags(CC_OP_UMUL);
    thread_ctx->clearExprFromReg(reg_l);
    if (reg_h != REG_INVALID())
      thread_ctx->clearExprFromReg(reg_h);
    return;
  }

  doMul(thread_ctx, ctx, reg_l, reg_h, expr_src, expr_reg_l, bits, sign);
}

void PIN_FAST_ANALYSIS_CALL
instrumentIMulReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG src) {
  REG reg_l = getAx(src);
  REG reg_h = getDx(src);
  instrumentMulReg(thread_ctx, ctx, src, reg_l, reg_h, true);
}

void PIN_FAST_ANALYSIS_CALL
instrumentIMulMem(ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG) {
  REG reg_l = getAx(size);
  REG reg_h = getDx(size);
  instrumentMulMem(thread_ctx, ctx,
      base, index, addr, size, disp, scale,
      reg_l, reg_h, true);
}

void PIN_FAST_ANALYSIS_CALL
instrumentIMulRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src) {
  instrumentMulReg(thread_ctx, ctx, src, dst, REG_INVALID(), true);
}

void PIN_FAST_ANALYSIS_CALL
instrumentIMulRegMem(ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG) {
  instrumentMulMem(thread_ctx, ctx,
      base, index, addr, size, disp, scale,
      dst, REG_INVALID(), true);
}

static void
doMul(ThreadContext* thread_ctx, ExprRef expr_src, ExprRef expr_imm, REG dst) {
  ExprRef expr_res = g_expr_builder->createBinaryExpr(Mul, expr_src, expr_imm);
  thread_ctx->setEflags(CC_OP_SMUL, expr_res, expr_src, expr_imm);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentIMulRegRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    ADDRINT imm) {
  ExprRef expr_src = NULL;
  ExprRef expr_imm = NULL;

  if (!getExprFromRegImm(
        thread_ctx,
        ctx,
        src,
        imm,
        &expr_src,
        &expr_imm)) {
    thread_ctx->clearExprFromReg(dst);
    thread_ctx->invalidateEflags(CC_OP_SMUL);
    return;
  }

  doMul(thread_ctx, expr_src, expr_imm, dst);
}

void PIN_FAST_ANALYSIS_CALL
instrumentIMulRegMemImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    ADDRINT imm) {
  ExprRef expr_src = NULL;
  ExprRef expr_imm = NULL;

  if (!getExprFromMemImm(
        thread_ctx,
        ctx,
        base, index, addr, size, disp, scale,
        imm,
        &expr_src,
        &expr_imm)) {
    thread_ctx->clearExprFromReg(dst);
    thread_ctx->invalidateEflags(CC_OP_SMUL);
    return;
  }

  doMul(thread_ctx, expr_src, expr_imm, dst);
}

void PIN_FAST_ANALYSIS_CALL
instrumentLeaRegMem(ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG base, REG index, ADDRINT disp, UINT32 scale) {
  ExprRef e = thread_ctx->getAddrExpr(ctx, base, index, disp, scale);
  thread_ctx->setExprToReg(dst, e);
}

static void
doMovRegReg(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    UINT dst_from,
    UINT src_from,
    UINT size) {
  for (UINT32 i = 0; i < size; i++) {
    ExprRef e = thread_ctx->getExprFromRegAlways(ctx, src,  i + src_from, 1);
    thread_ctx->setExprToReg(dst, e, i + dst_from, 1);
  }
}

void PIN_FAST_ANALYSIS_CALL
instrumentMovRegReg(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src) {
  doMovRegReg(thread_ctx, ctx, dst, src, 0, 0, REG_Size(dst));
}

void PIN_FAST_ANALYSIS_CALL
instrumentMovRegImm(ThreadContext *thread_ctx, const CONTEXT* ctx,
    REG dst, ADDRINT imm) {
  thread_ctx->clearExprFromReg(dst);
}

static void
doMovRegMem(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    UINT dst_from) {
  makeAddrConcrete(thread_ctx, ctx, base, index, addr, size, disp, scale);
  // if not a dereference symbol, copy in byte level
  for (UINT i = 0; i < size; i++) {
    ExprRef e = g_memory.getExprFromMemAlways(addr + i);
    thread_ctx->setExprToReg(dst, e, i + dst_from, 1);
  }
}

void PIN_FAST_ANALYSIS_CALL
instrumentMovRegMem(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG) {
  doMovRegMem(thread_ctx, ctx, dst,
    base, index, addr, size, disp, scale, 0);
}

void PIN_FAST_ANALYSIS_CALL
instrumentMovMemImm(ThreadContext *thread_ctx, const CONTEXT* ctx,
    MEM_ARG,
    ADDRINT imm) {
  makeAddrConcrete(thread_ctx, ctx, base, index, addr, size, disp, scale);
  g_memory.clearExprFromMem(addr, size);
}

static void
doMovMemReg(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG src,
    UINT src_from) {
  makeAddrConcrete(thread_ctx, ctx, base, index, addr, size, disp, scale);

  for (UINT i = 0; i < size; i++) {
    ExprRef e = thread_ctx->getExprFromRegAlways(ctx, src, i + src_from, 1);
    g_memory.setExprToMem(addr + i, e);
  }
}

void PIN_FAST_ANALYSIS_CALL
instrumentMovMemReg(ThreadContext *thread_ctx, const CONTEXT* ctx,
    MEM_ARG,
    REG src) {
  doMovMemReg(thread_ctx, ctx,
      base, index, addr, size, disp, scale,
      src, 0);
}

void PIN_FAST_ANALYSIS_CALL
instrumentMovhRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG) {
  QSYM_ASSERT(REG_Size(dst) == 16);
  doMovRegMem(thread_ctx, ctx, dst,
    base, index, addr, size, disp, scale,
    8);
}

void PIN_FAST_ANALYSIS_CALL
instrumentMovhMemReg(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG src) {
  QSYM_ASSERT(REG_Size(src) == 16);
  doMovMemReg(thread_ctx, ctx,
    base, index, addr, size, disp, scale,
    src, 8);
}

void PIN_FAST_ANALYSIS_CALL
instrumentMovlzRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    UINT width) {
  UINT size = REG_Size(src);
  doMovRegReg(thread_ctx, ctx, dst, src, 0, 0, width);
  thread_ctx->clearExprFromReg(dst, size, REG_Size(dst) - size);
}

void PIN_FAST_ANALYSIS_CALL
instrumentMovlzRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    UINT width) {
  doMovRegMem(thread_ctx, ctx, dst,
                base, index, addr, size, disp, scale, 0);
  QSYM_ASSERT(REG_Size(dst) >= size);
  thread_ctx->clearExprFromReg(dst, size, REG_Size(dst) - size);
}

void PIN_FAST_ANALYSIS_CALL
instrumentMovlzMemReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG src,
    UINT width) {
  doMovMemReg(thread_ctx, ctx,
      base, index, addr, size, disp, scale,
      src, 0);
}

void PIN_FAST_ANALYSIS_CALL
instrumentMovs(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    ADDRINT src_addr,
    ADDRINT dst_addr,
    UINT32 size) {
  // TODO: Add symbolic access in Movs*
  concretizeReg(thread_ctx, ctx, REG_GSI);
  concretizeReg(thread_ctx, ctx, REG_GDI);

  for (UINT32 i = 0; i < size; i++) {
    ExprRef e = g_memory.getExprFromMemAlways(src_addr + i);
    g_memory.setExprToMem(dst_addr + i, e);
  }
}

void PIN_FAST_ANALYSIS_CALL
instrumentMovsxRegMem(ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG) {
  // TODO: merge with movzx
  ExprRef e = getExprFromMemSrc(thread_ctx, ctx, base, index, addr, size, disp, scale);
  if (e != NULL)
    e = g_expr_builder->createSExt(e, REG_Size(dst) * 8);
  thread_ctx->setExprToReg(dst, e);
}

void PIN_FAST_ANALYSIS_CALL
instrumentMovsxRegReg(ThreadContext* thread_ctx,
  const CONTEXT* ctx,
  REG dst, REG src) {
  ExprRef e = thread_ctx->getExprFromReg(ctx, src);
  if (e != NULL)
    e = g_expr_builder->createSExt(e, REG_Size(dst) * 8);
  thread_ctx->setExprToReg(dst, e);
}

void PIN_FAST_ANALYSIS_CALL
instrumentMovzxRegMem(ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG) {
  ExprRef e = getExprFromMemSrc(thread_ctx, ctx, base, index, addr, size, disp, scale);
  if (e != NULL)
    e = g_expr_builder->createZExt(e, REG_Size(dst) * 8);
  thread_ctx->setExprToReg(dst, e);
}

void PIN_FAST_ANALYSIS_CALL
instrumentMovzxRegReg(ThreadContext* thread_ctx,
  const CONTEXT* ctx,
  REG dst, REG src) {
  ExprRef e = thread_ctx->getExprFromReg(ctx, src);
  if (e != NULL)
    e = g_expr_builder->createZExt(e, REG_Size(dst) * 8);
  thread_ctx->setExprToReg(dst, e);
}

void PIN_FAST_ANALYSIS_CALL
instrumentMulReg(ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG src, REG reg_l, REG reg_h) {
  instrumentMulReg(thread_ctx, ctx, src, reg_l, reg_h, false);
}

void PIN_FAST_ANALYSIS_CALL
instrumentMulMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG reg_l,
    REG reg_h) {
  instrumentMulMem(thread_ctx, ctx,
      base, index, addr, size, disp, scale,
      reg_l, reg_h, false);
}

static ExprRef
doNegNot(ThreadContext* thread_ctx, ExprRef expr_dst, Kind kind) {
  if (expr_dst == NULL) {
    thread_ctx->invalidateEflags(CC_OP_LOGIC);
    return NULL;
  }

  ExprRef expr_res = g_expr_builder->createUnaryExpr(kind, expr_dst);
  thread_ctx->setEflags(CC_OP_LOGIC, expr_res, expr_dst, NULL);
  return expr_res;
}

void PIN_FAST_ANALYSIS_CALL
instrumentNegNotReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    Kind kind) {
  ExprRef expr_dst = thread_ctx->getExprFromReg(ctx, dst);
  ExprRef expr_res = doNegNot(thread_ctx, expr_dst, kind);

  if (expr_res != NULL)
    thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentNegNotMem(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    Kind kind) {
  makeAddrConcrete(thread_ctx, ctx, base, index, addr, size, disp, scale);
  ExprRef expr_dst = g_memory.getExprFromMem(addr, size);
  ExprRef expr_res = doNegNot(thread_ctx, expr_dst, kind);

  if (expr_res != NULL)
    g_memory.setExprToMem(addr, size, expr_res);
}

void
instrumentJcc(ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    bool taken, ADDRINT target,
    JccKind jcc_c, bool inv) {
  ExprRef e = thread_ctx->computeJcc(ctx, jcc_c, inv);
  if (e) {
    ADDRINT pc = PIN_GetContextReg(ctx, REG_INST_PTR);
    LOG_DEBUG("Symbolic branch at " + hexstr(pc) + ": " + e->toString() + "\n");
#ifdef CONFIG_TRACE
    trace_addJcc(e, ctx, taken);
#endif
    g_solver->addJcc(e, taken, pc);
  }
}

void PIN_FAST_ANALYSIS_CALL
instrumentJmpReg(ThreadContext* thread_ctx, const CONTEXT* ctx, REG r) {
  ExprRef e = thread_ctx->getExprFromReg(ctx, r);
  if (e != NULL) {
    LOG_DEBUG("Symbolic jmp: " + e->toString() + "\n");
    llvm::APInt val = getRegValue(ctx, r);
    g_solver->solveAll(e, val);
    thread_ctx->clearExprFromReg(r);
  }
}

void PIN_FAST_ANALYSIS_CALL
instrumentJmpMem(ThreadContext* thread_ctx, const CONTEXT* ctx, MEM_ARG) {
  ExprRef e = g_memory.getExprFromMem(addr, size);
  if (e != NULL) {
    LOG_DEBUG("Symbolic jmp: " + e->toString() + "\n");
    llvm::APInt val = getMemValue(addr, size);
    g_solver->solveAll(e, val);
    g_memory.clearExprFromMem(addr, size);
  }
}

static ExprRef
saturateIntToInt(ExprRef e, UINT from, UINT to) {
  assert(from >= to);
  assert(e->bits() == from);

  llvm::APInt max_to = llvm::APInt::getSignedMaxValue(to);
  llvm::APInt min_to = llvm::APInt::getSignedMinValue(to);
  llvm::APInt max_from = max_to.sext(from);
  llvm::APInt min_from = min_to.sext(from);

  ExprRef expr_max_from = g_expr_builder->createConstant(max_from, from);
  ExprRef expr_min_from = g_expr_builder->createConstant(min_from, from);

  ExprRef expr_max_to = g_expr_builder->createConstant(max_to, to);
  ExprRef expr_min_to = g_expr_builder->createConstant(min_to, to);

  return g_expr_builder->createIte(
    g_expr_builder->createSlt(e, expr_min_from),
    expr_min_to,
    g_expr_builder->createIte(
      g_expr_builder->createSgt(e, expr_max_from),
      expr_max_to,
      g_expr_builder->createExtract(e, 0, to)));
}

static ExprRef
saturateIntToUint(ExprRef e, UINT from, UINT to) {
  assert(from >= to);
  assert(e->bits() == from);

  llvm::APInt max_to = llvm::APInt::getMaxValue(to);
  llvm::APInt min_to = llvm::APInt::getMinValue(to);
  llvm::APInt max_from = max_to.zext(from);
  llvm::APInt min_from = min_to.zext(from);

  ExprRef expr_max_from = g_expr_builder->createConstant(max_from, from);
  ExprRef expr_min_from = g_expr_builder->createConstant(min_from, from);

  ExprRef expr_max_to = g_expr_builder->createConstant(max_to, to);
  ExprRef expr_min_to = g_expr_builder->createConstant(min_to, to);

  return g_expr_builder->createIte(
    g_expr_builder->createUlt(e, expr_min_from),
    expr_min_to,
    g_expr_builder->createIte(
      g_expr_builder->createUgt(e, expr_max_from),
      expr_max_to,
      g_expr_builder->createExtract(e, 0, to)));
}

static void
doPack(
    ThreadContext *thread_ctx,
    REG dst,
    ExprRef expr_dst,
    ExprRef expr_src,
    UINT width,
    bool sign) {
  UINT bits = REG_Size(dst) * CHAR_BIT;
  width *= CHAR_BIT;
  std::list<ExprRef> exprs;
  ExprRef expr_concat = g_expr_builder->createConcat(expr_src, expr_dst);

  for (UINT i = 0; i < bits; i += width) {
    ExprRef expr_sub = g_expr_builder->createExtract(
        expr_concat, i * 2, width * 2);
    if (sign)
      expr_sub = saturateIntToInt(expr_sub, width * 2, width);
    else
      expr_sub = saturateIntToUint(expr_sub, width * 2, width);
    exprs.push_front(expr_sub);
  }

  ExprRef expr_res = g_expr_builder->createConcat(exprs);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPackRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    UINT width,
    bool sign) {
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegReg(
        thread_ctx,
        ctx,
        dst,
        src,
        &expr_dst,
        &expr_src))
    return;

  doPack(thread_ctx, dst, expr_dst, expr_src,
      width, sign);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPackRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    UINT width,
    bool sign) {
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegMem(
        thread_ctx,
        ctx,
        dst,
        base, index, addr, size, disp, scale,
        &expr_dst,
        &expr_src))
    return;

  doPack(thread_ctx, dst, expr_dst, expr_src,
      width, sign);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPalignrReg(ThreadContext* thread_ctx, const CONTEXT* ctx,
    REG dst, REG src, ADDRINT off) {
  INT32 bits = REG_Size(dst) * CHAR_BIT;
  ExprRef expr_dst = thread_ctx->getExprFromReg(ctx, dst);
  ExprRef expr_src = thread_ctx->getExprFromReg(ctx, src);

  if (expr_src == NULL && expr_dst == NULL)
    return ;

  fixRegExpr(&expr_src, ctx, src);
  fixRegExpr(&expr_dst, ctx, dst);

  ExprRef e = g_expr_builder->createConcat(expr_dst, expr_src);
  e = g_expr_builder->createExtract(e,
      off * CHAR_BIT,
      bits);
  thread_ctx->setExprToReg(dst, e);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPalignrMem(ThreadContext* thread_ctx, const CONTEXT* ctx,
    REG dst, MEM_ARG, ADDRINT off) {
  makeAddrConcrete(thread_ctx, ctx, base, index, addr, size, disp, scale);

  INT32 bits = size * CHAR_BIT;
  ExprRef expr_src = g_memory.getExprFromMem(addr, size);
  ExprRef expr_dst = thread_ctx->getExprFromReg(ctx, dst);

  if (expr_src == NULL && expr_dst == NULL)
    return;

  fixMemExpr(&expr_src, addr, size);
  fixRegExpr(&expr_dst, ctx, dst);

  ExprRef e = g_expr_builder->createConcat(expr_dst, expr_src);
  e = g_expr_builder->createExtract(e,
      off * CHAR_BIT,
      bits);
  thread_ctx->setExprToReg(dst, e);
}

static void
doParith(
    ThreadContext *thread_ctx,
    REG dst,
    ExprRef expr_dst,
    ExprRef expr_src,
    Kind kind,
    UINT width) {
  std::list<ExprRef> exprs;
  UINT bits = REG_Size(dst) * CHAR_BIT;
  width *= CHAR_BIT;

  for (UINT i = 0; i < bits; i += width) {
    ExprRef expr_sub_dst = g_expr_builder->createExtract(
        expr_dst, i, width);
    ExprRef expr_sub_src = g_expr_builder->createExtract(
        expr_src, i, width);
    ExprRef expr_sub_res = g_expr_builder->createBinaryExpr(
        kind, expr_sub_dst, expr_sub_src);
    exprs.push_front(expr_sub_res);
  }

  ExprRef expr_res = g_expr_builder->createConcat(exprs);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentParithRegReg(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    Kind kind,
    UINT width) {
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegReg(thread_ctx, ctx, dst, src, &expr_dst, &expr_src))
    return;

  doParith(thread_ctx, dst, expr_dst, expr_src, kind, width);
}

void PIN_FAST_ANALYSIS_CALL
instrumentParithRegMem(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    Kind kind,
    UINT width) {
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegMem(
        thread_ctx, ctx, dst,
        base, index, addr, size, disp, scale,
        &expr_dst, &expr_src))
      return;

  doParith(thread_ctx, dst, expr_dst, expr_src, kind, width);
}

static void
doPcmp(
    ThreadContext* thread_ctx,
    REG dst,
    ExprRef expr_dst,
    ExprRef expr_src,
    Kind kind,
    UINT32 width) {
  std::list<ExprRef> exprs;
  UINT32 bits = REG_Size(dst) * CHAR_BIT;
  width *= CHAR_BIT;

  llvm::APInt ff(width, 0);
  ff.setAllBits();

  ExprRef success = g_expr_builder->createConstant(ff, width);
  ExprRef fail = g_expr_builder->createConstant(0, width);

  for (UINT32 i = 0; i < bits; i += width) {
    ExprRef expr_dst_sub = g_expr_builder->createExtract(expr_dst, i, width);
    ExprRef expr_src_sub = g_expr_builder->createExtract(expr_src, i, width);
    ExprRef expr_res_sub = g_expr_builder->createIte(
        g_expr_builder->createBinaryExpr(kind, expr_dst_sub, expr_src_sub),
        success,
        fail);
    exprs.push_front(expr_res_sub);
  }

  ExprRef expr_res = g_expr_builder->createConcat(exprs);
  thread_ctx->setExprToReg(dst, expr_res);
}

void
instrumentPcmpRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    Kind kind,
    UINT width) {
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegReg(thread_ctx, ctx, dst, src, &expr_dst, &expr_src))
    return;

  doPcmp(thread_ctx, dst, expr_dst, expr_src, kind, width);
}

void
instrumentPcmpRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    Kind kind,
    UINT width) {
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegMem(
        thread_ctx, ctx, dst,
        base, index, addr, size, disp, scale,
        &expr_dst, &expr_src))
    return;

  doPcmp(thread_ctx, dst, expr_dst, expr_src, kind, width);
}

static ExprRef
doPextr(
    ThreadContext *thread_ctx,
    ExprRef expr_src,
    ADDRINT imm,
    UINT bits,
    UINT width) {
  // convert into bits
  width *= CHAR_BIT;

  // normalize, bits / width - 1 -> number of maximum bits that we can move
  // * width --> shift imm with 'width' bits
  imm = imm & (bits / width - 1) * width;

  return g_expr_builder->createZExt(
      g_expr_builder->createExtract(
        g_expr_builder->createAShr(expr_src,
          g_expr_builder->createConstant(imm, bits)),
        0, width),
      bits);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPextrRegRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    ADDRINT imm,
    UINT width) {
  ExprRef expr_src = thread_ctx->getExprFromReg(ctx, src);
  if (expr_src == NULL) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  ExprRef expr_res = doPextr(thread_ctx, expr_src, imm,
      REG_Size(dst) * CHAR_BIT, width);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPextrMemRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    REG src,
    ADDRINT imm,
    UINT width) {
  makeAddrConcrete(thread_ctx, ctx, base, index, addr, size, disp, scale);
  ExprRef expr_src = thread_ctx->getExprFromReg(ctx, src);
  if (expr_src == NULL) {
    g_memory.clearExprFromMem(addr, size);
    return;
  }

  ExprRef expr_res = doPextr(thread_ctx, expr_src, imm,
      size * CHAR_BIT, width);
  g_memory.setExprToMem(addr, size, expr_res);
}

static void
doPmaddwd(
    ThreadContext *thread_ctx,
    REG dst,
    ExprRef expr_dst,
    ExprRef expr_src) {
  UINT size = REG_Size(dst);
  ExprRef exprs_mul[size / 2];
  std::list<ExprRef> exprs;

  for (UINT i = 0; i < size; i += 2) {
    ExprRef expr_sub_dst = g_expr_builder->createExtract(
        expr_dst, i * CHAR_BIT, 2 * CHAR_BIT);
    ExprRef expr_sub_src = g_expr_builder->createExtract(
        expr_src, i * CHAR_BIT, 2 * CHAR_BIT);

    expr_sub_dst = g_expr_builder->createZExt(expr_sub_dst, 4 * CHAR_BIT);
    expr_sub_src = g_expr_builder->createZExt(expr_sub_src, 4 * CHAR_BIT);

    exprs_mul[i / 2] = g_expr_builder->createMul(expr_sub_dst, expr_sub_src);
  }

  for (UINT i = 0; i < size; i += 4) {
    ExprRef expr_mul1 = exprs_mul[i / 2];
    ExprRef expr_mul2 = exprs_mul[i / 2 + 1];

    ExprRef expr_sub_res = g_expr_builder->createAdd(expr_mul1, expr_mul2);
    exprs.push_front(expr_sub_res);
  }

  ExprRef expr_res = g_expr_builder->createConcat(exprs);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPmaddwdRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src) {
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegReg(
        thread_ctx,
        ctx,
        dst,
        src,
        &expr_dst,
        &expr_src))
    return;

  doPmaddwd(thread_ctx, dst, expr_dst, expr_src);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPmaddwdRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG) {
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegMem(
        thread_ctx, ctx, dst,
        base, index, addr, size, disp, scale,
        &expr_dst, &expr_src))
    return;

  doPmaddwd(thread_ctx, dst, expr_dst, expr_src);
}

static void
doPcmpxchg(
    ThreadContext* thread_ctx,
    REG dst,
    ExprRef expr_dst,
    ExprRef expr_src,
    Kind kind,
    UINT width) {
  std::list<ExprRef> exprs;
  UINT bits = REG_Size(dst) * CHAR_BIT;
  width *= CHAR_BIT;

  for (UINT i = 0; i < bits; i += width) {
    ExprRef sub_dst = g_expr_builder->createExtract(expr_dst, i, width);
    ExprRef sub_src = g_expr_builder->createExtract(expr_src, i, width);
    ExprRef sub_res = g_expr_builder->createIte(
        g_expr_builder->createBinaryExpr(kind, sub_dst, sub_src),
        sub_dst,
        sub_src);
    exprs.push_front(sub_res);
  }

  ExprRef expr_res = g_expr_builder->createConcat(exprs);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPcmpxchgRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    Kind kind,
    UINT width) {
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegReg(
        thread_ctx,
        ctx,
        dst,
        src,
        &expr_dst,
        &expr_src))
    return;

  doPcmpxchg(thread_ctx, dst, expr_dst, expr_src, kind, width);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPcmpxchgRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    Kind kind,
    UINT width) {
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegMem(
        thread_ctx, ctx, dst,
        base, index, addr, size, disp, scale,
        &expr_dst, &expr_src))
    return;

  doPcmpxchg(thread_ctx, dst, expr_dst, expr_src, kind, width);
}

static void
doPmul(
  ThreadContext *thread_ctx,
  REG dst,
  ExprRef expr_dst,
  ExprRef expr_src,
  UINT width) {
  std::list<ExprRef> exprs;
  UINT bits = REG_Size(dst) * CHAR_BIT;
  width *= CHAR_BIT;

  for (UINT i = 0; i < bits; i += width) {
    ExprRef expr_sub_dst = g_expr_builder->createExtract(
        expr_dst, i, width);
    ExprRef expr_sub_src = g_expr_builder->createExtract(
        expr_src, i, width);
    ExprRef expr_sub_res = g_expr_builder->createMul(
        expr_sub_dst, expr_sub_src);
    exprs.push_front(expr_sub_res);
  }

  ExprRef expr_res = g_expr_builder->createConcat(exprs);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPmulRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    UINT width) {
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegReg(thread_ctx, ctx, dst, src, &expr_dst, &expr_src))
    return;

  doPmul(thread_ctx, dst, expr_dst, expr_src, width);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPmulRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    UINT width) {
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegMem(thread_ctx, ctx, dst,
        base, index, addr, size, disp, scale,
        &expr_dst, &expr_src))
    return;

  doPmul(thread_ctx, dst, expr_dst, expr_src, width);
}

static void
doPmulhw(
  ThreadContext* thread_ctx,
  REG dst,
  ExprRef expr_dst,
  ExprRef expr_src,
  bool sign) {
  std::list<ExprRef> exprs;
  UINT size = REG_Size(dst);
  for (UINT i = 0; i < size; i += 2) {
    ExprRef expr_sub_dst = g_expr_builder->createExtract(
        expr_dst, i * CHAR_BIT, 2 * CHAR_BIT);
    ExprRef expr_sub_src = g_expr_builder->createExtract(
        expr_src, i * CHAR_BIT, 2 * CHAR_BIT);
    expr_sub_dst = extendToDouble(expr_sub_dst, sign);
    expr_sub_src = extendToDouble(expr_sub_src, sign);

    ExprRef expr_sub_res = g_expr_builder->createMul(
        expr_sub_dst, expr_sub_src);
    expr_sub_res = g_expr_builder->createExtract(expr_sub_res, 2 * CHAR_BIT, 2 * CHAR_BIT);
    exprs.push_front(expr_sub_res);
  }

  ExprRef expr_res = g_expr_builder->createConcat(exprs);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPmulhwRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    bool sign) {
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegReg(thread_ctx, ctx, dst, src, &expr_dst, &expr_src))
    return;

  doPmulhw(thread_ctx, dst, expr_dst, expr_src, sign);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPmulhwRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    bool sign) {
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegMem(thread_ctx, ctx, dst,
        base, index, addr, size, disp, scale,
        &expr_dst, &expr_src))
    return;

  doPmulhw(thread_ctx, dst, expr_dst, expr_src, sign);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPmovmskb(ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst, REG src)
{
  ExprRef expr_src = thread_ctx->getExprFromReg(ctx, src);
  std::list<ExprRef> exprs;

  if (expr_src == NULL) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  ExprRef expr_dst = NULL;
  for (UINT i = 0; i < REG_Size(src); i++) {
    exprs.push_front(
        g_expr_builder->createExtract(
          expr_src,
          7 + i * CHAR_BIT,
          1)
    );
  }

  expr_dst = g_expr_builder->createZExt(
              g_expr_builder->createConcat(exprs),
              REG_Size(dst) * CHAR_BIT);
  thread_ctx->setExprToReg(dst, expr_dst);
}

static void
doPshuf(
    ThreadContext* thread_ctx,
    REG dst,
    ExprRef expr_src,
    ADDRINT imm,
    UINT size,
    UINT width,
    UINT beg=0) {
  // DEST[31:0] ← (SRC[127:0] >> (ORDER[1:0] * 32))[31:0];
  // DEST[63:32] ← (SRC[127:0] >> (ORDER[3:2] * 32))[31:0];
  // DEST[95:64] ← (SRC[127:0] >> (ORDER[5:4] * 32))[31:0];
  // DEST[127:96] ← (SRC[127:0] >> (ORDER[7:6] * 32))[31:0];
  // DEST[159:128] ← (SRC[255:128] >> (ORDER[1:0] * 32))[31:0];
  // DEST[191:160] ← (SRC[255:128] >> (ORDER[3:2] * 32))[31:0];
  // DEST[223:192] ← (SRC[255:128] >> (ORDER[5:4] * 32))[31:0];
  // DEST[255:224] ← (SRC[255:128] >> (ORDER[7:6] * 32))[31:0];

  std::list<ExprRef> exprs;
  for (UINT i = 0; i < size / width; i++) {
    ExprRef expr_const = g_expr_builder->createConstant(
        ((imm >> (2 * i)) & 3) * width * CHAR_BIT + (size / 128) * 128 + beg,
        expr_src->bits());
    ExprRef expr_sub_res = g_expr_builder->createLShr(expr_src, expr_const);
    expr_sub_res = g_expr_builder->createExtract(expr_sub_res, 0, 32);
    exprs.push_front(expr_sub_res);
  }

  ExprRef expr_res = g_expr_builder->createConcat(exprs);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPshufRegRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    ADDRINT imm,
    UINT width) {
  ExprRef expr_src = thread_ctx->getExprFromReg(ctx, src);
  if (expr_src == NULL) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  doPshuf(thread_ctx, dst, expr_src, imm, REG_Size(dst), width);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPshufRegMemImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    ADDRINT imm,
    UINT width) {
  ExprRef expr_src = getExprFromMemSrc(thread_ctx, ctx, base, index, addr, size,
                                       disp, scale);
  if (expr_src == NULL) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  doPshuf(thread_ctx, dst, expr_src, imm, size, width);
}

static void
doPshufb(
    ThreadContext *thread_ctx,
    REG dst,
    ExprRef expr_dst,
    ExprRef expr_src) {
  UINT size = REG_Size(dst);
  UINT bits = size * CHAR_BIT;
  std::list<ExprRef> exprs;
  ExprRef expr_shift = NULL;
  ExprRef expr_one_bit = g_expr_builder->createConstant(1, 1);
  ExprRef expr_zero_byte = g_expr_builder->createConstant(0, 8);
  ExprRef expr_three = g_expr_builder->createConstant(3, bits);

  ExprRef expr_mask = NULL;
  if (size == 8)
    expr_mask = g_expr_builder->createConstant((1 << 3) - 1, CHAR_BIT);
  else
    expr_mask = g_expr_builder->createConstant((1 << 4) - 1, CHAR_BIT);

  for (UINT i = 0; i < size; i++) {
    ExprRef expr_msb = g_expr_builder->createExtract(expr_src, i * CHAR_BIT + 7, 1);
    ExprRef expr_index = g_expr_builder->createShl(
                          g_expr_builder->createZExt(
                            g_expr_builder->createAnd(
                              g_expr_builder->createExtract(
                                expr_src, i * CHAR_BIT, CHAR_BIT),
                              expr_mask),
                            bits),
                          expr_three);
    // for vpshufb
    if (i >= 16) {
      expr_index = g_expr_builder->createAdd(
          expr_index,
          g_expr_builder->createConstant(128, bits));
    }

    ExprRef expr_false = g_expr_builder->createExtract(
                            g_expr_builder->createLShr(expr_dst, expr_index),
                            0, CHAR_BIT);
    ExprRef expr_ite = g_expr_builder->createIte(
      g_expr_builder->createEqual(expr_msb, expr_one_bit),
      expr_zero_byte,
      expr_false
    );

    exprs.push_front(expr_ite);
  }

  ExprRef expr_res = g_expr_builder->createConcat(exprs);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPshufbRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src) {
  // TODO: add non-linear operation
  concretizeReg(thread_ctx, ctx, src);

  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegReg(
      thread_ctx,
      ctx,
      dst,
      src,
      &expr_dst,
      &expr_src))
    return;

  doPshufb(thread_ctx, dst, expr_dst, expr_src);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPshufbRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG) {
  // TODO: add non-linear operation
  concretizeMem(ctx, addr, size);

  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegMem(
      thread_ctx,
      ctx,
      dst,
      base, index, addr, size, disp, scale,
      &expr_dst,
      &expr_src))
    return;

  doPshufb(thread_ctx, dst, expr_dst, expr_src);
}

static void
doPshift(
    ThreadContext* thread_ctx,
    REG dst,
    Kind kind,
    ExprRef expr_dst,
    ADDRINT imm,
    UINT width) {
  UINT size = REG_Size(dst);
  // for pslldq & psrldq
  if (width == 16)
    imm = imm * CHAR_BIT;

  ExprRef expr_const = g_expr_builder->createConstant(imm, width * CHAR_BIT);
  std::list<ExprRef> exprs;

  for (UINT i = 0; i < size; i+= width) {
    ExprRef expr_sub_dst
      = g_expr_builder->createExtract(
          expr_dst,
          i * CHAR_BIT,
          width * CHAR_BIT);

    expr_sub_dst = g_expr_builder->createBinaryExpr(kind, expr_sub_dst, expr_const);
    exprs.push_front(expr_sub_dst);
  }

  ExprRef expr_res = g_expr_builder->createConcat(exprs);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPshiftRegReg(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    Kind kind,
    UINT width) {
  ExprRef expr_dst = thread_ctx->getExprFromReg(ctx, dst);

  // TODO: support non-linear operation
  concretizeReg(thread_ctx, ctx, src);

  if (expr_dst == NULL)
    return;

  llvm::APInt value = getRegValue(ctx, src);
  doPshift(thread_ctx, dst,
      kind, expr_dst, value.getLimitedValue(), width);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPshiftRegImm(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    ADDRINT imm,
    Kind kind,
    UINT width) {
  ExprRef expr_dst = thread_ctx->getExprFromReg(ctx, dst);

  if (expr_dst == NULL)
    return;

  doPshift(thread_ctx, dst, kind, expr_dst, imm, width);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPshiftRegMem(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    Kind kind,
    UINT width) {
  ExprRef expr_dst = thread_ctx->getExprFromReg(ctx, dst);

  // TODO: support non-linear operation
  makeAddrConcrete(thread_ctx, ctx, base, index, addr, size, disp, scale);
  if (expr_dst == NULL)
    return;

  llvm::APInt value = getMemValue(addr, size);
  doPshift(thread_ctx, dst, kind, expr_dst, value.getLimitedValue(), width);
}

static void
doPunpckh(
    ThreadContext* thread_ctx,
    REG dst,
    ExprRef expr_dst,
    ExprRef expr_src,
    UINT width,
    UINT size) {
  std::list<ExprRef> exprs;

  for (UINT i = 0; i < size; i += width * 2) {
    exprs.push_front(
          g_expr_builder->createExtract(expr_dst,
            (i / 2 + size / 2) * CHAR_BIT,  width * CHAR_BIT)
          );
    exprs.push_front(
          g_expr_builder->createExtract(expr_src,
            (i / 2 + size / 2) * CHAR_BIT,  width * CHAR_BIT)
          );
  }

  ExprRef expr_res = g_expr_builder->createConcat(exprs);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPunpckhRegReg(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    UINT width) {
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegReg(thread_ctx, ctx, dst, src, &expr_dst, &expr_src))
    return;

  doPunpckh(thread_ctx, dst, expr_dst, expr_src, width, REG_Size(dst));
}

void PIN_FAST_ANALYSIS_CALL
instrumentPunpckhRegMem(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    UINT width) {
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegMem(thread_ctx, ctx, dst,
        base, index, addr, size, disp, scale,
        &expr_dst, &expr_src))
    return;

  doPunpckh(thread_ctx, dst, expr_dst, expr_src, width, size);
}

static void
doPunpckl(
    ThreadContext *thread_ctx,
    REG dst,
    ExprRef expr_dst,
    ExprRef expr_src,
    UINT width,
    UINT size) {
  std::list<ExprRef> exprs;
  for (UINT i = 0; i < size; i += width * 2) {
    exprs.push_front(
          g_expr_builder->createExtract(expr_dst, (i / 2) * CHAR_BIT,  width * CHAR_BIT)
    );
    exprs.push_front(
          g_expr_builder->createExtract(expr_src, (i / 2) * CHAR_BIT,  width * CHAR_BIT)
    );
  }

  ExprRef expr_res = g_expr_builder->createConcat(exprs);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPunpcklRegReg(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    UINT width) {
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegReg(thread_ctx, ctx, dst, src, &expr_dst, &expr_src))
    return;

  doPunpckl(thread_ctx, dst, expr_dst, expr_src, width, REG_Size(dst));
}

void PIN_FAST_ANALYSIS_CALL
instrumentPunpcklRegMem(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    UINT width) {
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegMem(thread_ctx, ctx, dst,
        base, index, addr, size, disp, scale,
        &expr_dst, &expr_src))
    return;

  doPunpckl(thread_ctx, dst, expr_dst, expr_src, width, size);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPushReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    ADDRINT dst_addr,
    UINT dst_size,
    REG src) {
  // keep stack non-symbolic
  concretizeReg(thread_ctx, ctx, REG_STACK_PTR);

  for (UINT i = 0; i < dst_size; i++) {
    ExprRef e = thread_ctx->getExprFromReg(ctx, src, i, 1);
    g_memory.setExprToMem(dst_addr + i, e);
  }
}

void PIN_FAST_ANALYSIS_CALL
instrumentPushImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    ADDRINT dst_addr,
    UINT dst_size) {
  // keep stack non-symbolic
  concretizeReg(thread_ctx, ctx, REG_STACK_PTR);
  g_memory.clearExprFromMem(dst_addr, dst_size);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPushMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    ADDRINT dst_addr,
    MEM_ARG) {
  // keep stack non-symbolic
  concretizeReg(thread_ctx, ctx, REG_STACK_PTR);
  makeAddrConcrete(thread_ctx, ctx, base, index, addr, size, disp, scale);

  for (UINT i = 0; i < size; i++) {
    ExprRef e = g_memory.getExprFromMem(addr + i);
    g_memory.setExprToMem(dst_addr + i, e);
  }
}

void PIN_FAST_ANALYSIS_CALL
instrumentPopReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    ADDRINT src_addr,
    UINT src_size) {
  // keep stack non-symbolic
  concretizeReg(thread_ctx, ctx, REG_STACK_PTR);
  for (UINT i = 0; i < src_size; i++) {
    ExprRef e = g_memory.getExprFromMem(src_addr + i);
    thread_ctx->setExprToReg(dst, e, i, 1);
  }
}

void PIN_FAST_ANALYSIS_CALL
instrumentPopMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    ADDRINT src_addr) {
  // keep stack non-symbolic
  concretizeReg(thread_ctx, ctx, REG_STACK_PTR);
  makeAddrConcrete(thread_ctx, ctx, base, index, addr, size, disp, scale);

  for (UINT i = 0; i < size; i++) {
    ExprRef e = g_memory.getExprFromMem(src_addr + i);
    g_memory.setExprToMem(addr + i, e);
  }
}


void PIN_FAST_ANALYSIS_CALL
instrumentRdtsc(ThreadContext* thread_ctx) {
  thread_ctx->clearExprFromReg(REG_EAX);
  thread_ctx->clearExprFromReg(REG_EDX);
}

void PIN_FAST_ANALYSIS_CALL
instrumentRdtscp(ThreadContext* thread_ctx) {
  thread_ctx->clearExprFromReg(REG_EAX);
  thread_ctx->clearExprFromReg(REG_EDX);
  thread_ctx->clearExprFromReg(REG_ECX);
}

void PIN_FAST_ANALYSIS_CALL
instrumentScas(
    ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    ADDRINT addr,
    UINT size) {
  // TODO: add symbolic access
  concretizeReg(thread_ctx, ctx, REG_GDI);

  REG ax = getAx(size);
  ExprRef expr_ax = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegMem(
        thread_ctx,
        ctx,
        ax,
        addr, size,
        &expr_ax,
        &expr_src)) {
    thread_ctx->invalidateEflags(CC_OP_SUB);
    return;
  }

  ExprRef expr_res = g_expr_builder->createBinaryExpr(Sub, expr_ax, expr_src);
  thread_ctx->setEflags(CC_OP_SUB, expr_res, expr_ax, expr_src);
}

static ExprRef
doSet(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    JccKind jcc_c,
    bool inv) {
  return thread_ctx->computeJccAsBV(ctx, jcc_c, inv, 8);
}

void PIN_FAST_ANALYSIS_CALL
instrumentSetReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    JccKind jcc_c,
    bool inv) {
  ExprRef expr_res = doSet(thread_ctx, ctx, jcc_c, inv);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentSetMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    MEM_ARG,
    JccKind jcc_c,
    bool inv) {
  makeAddrConcrete(thread_ctx, ctx, base, index, addr, size, disp, scale);
  ExprRef expr_res = doSet(thread_ctx, ctx, jcc_c, inv);
  g_memory.setExprToMem(addr, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentStos(ThreadContext *thread_ctx,
    const CONTEXT* ctx,
    ADDRINT dst_addr,
    REG src) {
  UINT32 size = REG_Size(src);
  concretizeReg(thread_ctx, ctx, REG_GDI);

  for (UINT32 i = 0; i < size; i++) {
    ExprRef e = thread_ctx->getExprFromReg(ctx, src, i, 1);
    g_memory.setExprToMem(dst_addr + i, e);
  }
}

void PIN_FAST_ANALYSIS_CALL
instrumentXaddRegReg(ThreadContext* thread_ctx, const CONTEXT *ctx,
    REG dst, REG src) {
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromRegReg(
        thread_ctx,
        ctx,
        dst,
        src,
        &expr_dst,
        &expr_src))
    return;

  // temporary = source + destination;
  // source = destination;
  // destination = temporary;

  ExprRef expr_tmp = g_expr_builder->createBinaryExpr(Add, expr_src, expr_dst);
  thread_ctx->setExprToReg(dst, expr_tmp);
  thread_ctx->setExprToReg(src, expr_dst);
}

void PIN_FAST_ANALYSIS_CALL
instrumentXaddMemReg(ThreadContext* thread_ctx, const CONTEXT *ctx,
    MEM_ARG, REG src) {
  ExprRef expr_dst = NULL;
  ExprRef expr_src = NULL;

  if (!getExprFromMemReg(
        thread_ctx,
        ctx,
        base, index, addr, size, disp, scale,
        src,
        &expr_dst,
        &expr_src))
    return;

  ExprRef expr_tmp = g_expr_builder->createBinaryExpr(Add, expr_src, expr_dst);
  g_memory.setExprToMem(addr, size, expr_tmp);
  thread_ctx->setExprToReg(src, expr_dst);
}

void PIN_FAST_ANALYSIS_CALL
instrumentXchgRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT *ctx,
    REG dst,
    REG src) {
  ExprRef expr_src = thread_ctx->getExprFromReg(ctx, src);
  ExprRef expr_dst = thread_ctx->getExprFromReg(ctx, dst);

  thread_ctx->setExprToReg(dst, expr_src);
  thread_ctx->setExprToReg(src, expr_dst);
}

void PIN_FAST_ANALYSIS_CALL
instrumentXchgRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT *ctx,
    REG dst,
    MEM_ARG) {
  ExprRef expr_src = g_memory.getExprFromMem(addr, size);
  ExprRef expr_dst = thread_ctx->getExprFromReg(ctx, dst);

  g_memory.setExprToMem(addr, size, expr_dst);
  thread_ctx->setExprToReg(dst, expr_src);
}

void PIN_FAST_ANALYSIS_CALL
instrumentXchgMemReg(
    ThreadContext* thread_ctx,
    const CONTEXT *ctx,
    MEM_ARG,
    REG src) {
  ExprRef expr_dst = g_memory.getExprFromMem(addr, size);
  ExprRef expr_src = thread_ctx->getExprFromReg(ctx, src);

  thread_ctx->setExprToReg(src, expr_dst);
  g_memory.setExprToMem(addr, size, expr_src);
}

// default instrumentation
void PIN_FAST_ANALYSIS_CALL
instrumentConcretizeReg(ThreadContext* thread_ctx, const CONTEXT *ctx, REG r) {
  concretizeReg(thread_ctx, ctx, r);
}

void PIN_FAST_ANALYSIS_CALL
instrumentConcretizeMem(const CONTEXT*ctx, ADDRINT addr, INT32 size) {
  concretizeMem(ctx, addr, size);
}

void PIN_FAST_ANALYSIS_CALL
instrumentConcretizeEflags(ThreadContext* thread_ctx,
    const CONTEXT *ctx) {
  // NOTE: It may lose correctness
  // For example, if EFLAGS is symbolic and an instruction that
  // triggers this function makes EFLAGS invalid. Then, if
  // the next instruction uses a bit from EFLAGS which is
  // not set by the instruction. Then, it is actually symbolic
  // but it will be considered as concrete.
  thread_ctx->invalidateEflags(CC_OP_ADD);
}

void PIN_FAST_ANALYSIS_CALL
instrumentClrReg(ThreadContext* thread_ctx,
    const CONTEXT* ctx, REG r) {
  thread_ctx->clearExprFromReg(r);
}

void PIN_FAST_ANALYSIS_CALL
instrumentClrMem(ADDRINT addr, INT32 size) {
  g_memory.clearExprFromMem(addr, size);
}

void
PIN_FAST_ANALYSIS_CALL
instrumentTernaryRegRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2,
    Kind kind) {
  ExprRef expr_src1 = NULL;
  ExprRef expr_src2 = NULL;

  if (!getExprFromRegReg(
        thread_ctx,
        ctx,
        src1,
        src2,
        &expr_src1,
        &expr_src2)) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  ExprRef expr_res = g_expr_builder->createBinaryExpr(kind, expr_src1, expr_src2);
  thread_ctx->setExprToReg(dst, expr_res);
}

void
PIN_FAST_ANALYSIS_CALL
instrumentTernaryRegRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    MEM_ARG,
    Kind kind) {
  ExprRef expr_src1 = NULL;
  ExprRef expr_src2 = NULL;

  if (!getExprFromRegMem(
        thread_ctx,
        ctx,
        src1,
        base, index, addr, size, disp, scale,
        &expr_src1,
        &expr_src2)) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  ExprRef expr_res = g_expr_builder->createBinaryExpr(kind, expr_src1, expr_src2);
  thread_ctx->setExprToReg(dst, expr_res);
}


static ExprRef
doVinsert128i(ExprRef expr_src1, ExprRef expr_src2, ADDRINT imm) {
  ExprRef expr_res = NULL;

  QSYM_ASSERT(expr_src1->bits() == 256
      && expr_src2->bits() == 128);

  if ((imm & 1) == 0) {
    expr_res = g_expr_builder->createConcat(
        g_expr_builder->createExtract(expr_src1, 128, 128),
        expr_src2);
  }
  else {
    expr_res = g_expr_builder->createConcat(
        expr_src2,
        g_expr_builder->createExtract(expr_src1, 0, 128));
  }

  return expr_res;
}

void PIN_FAST_ANALYSIS_CALL
instrumentVmovlRegRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    MEM_ARG) {
  QSYM_ASSERT(REG_Size(dst) == 16);
  doMovRegReg(thread_ctx, ctx, dst, src, 8, 8, 8);
  doMovRegMem(thread_ctx, ctx, dst,
      base, index, addr, size, disp, scale, 0);
}

void PIN_FAST_ANALYSIS_CALL
instrumentVparithRegRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2,
    Kind kind,
    UINT width) {
  ExprRef expr_src1 = NULL;
  ExprRef expr_src2 = NULL;

  if (!getExprFromRegReg(thread_ctx, ctx, src1, src2, &expr_src1, &expr_src2)) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  doParith(thread_ctx, dst, expr_src1, expr_src2, kind, width);
}

void PIN_FAST_ANALYSIS_CALL
instrumentVparithRegRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    MEM_ARG,
    Kind kind,
    UINT width) {
  ExprRef expr_src1 = NULL;
  ExprRef expr_src2 = NULL;

  if (!getExprFromRegMem(
        thread_ctx, ctx, src1,
        base, index, addr, size, disp, scale,
        &expr_src1, &expr_src2)) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  doParith(thread_ctx, dst, expr_src1, expr_src2, kind, width);
}

void PIN_FAST_ANALYSIS_CALL
instrumentVpcmpRegRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2,
    Kind kind,
    UINT width) {
  ExprRef expr_src1 = NULL;
  ExprRef expr_src2 = NULL;

  if (!getExprFromRegReg(thread_ctx, ctx, src1, src2, &expr_src1, &expr_src2)) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  doPcmp(thread_ctx, dst, expr_src1, expr_src2, kind, width);
}

void PIN_FAST_ANALYSIS_CALL
instrumentVpcmpRegRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    MEM_ARG,
    Kind kind,
    UINT width) {
  ExprRef expr_src1 = NULL;
  ExprRef expr_src2 = NULL;

  if (!getExprFromRegMem(
        thread_ctx, ctx, src1,
        base, index, addr, size, disp, scale,
        &expr_src1, &expr_src2)) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  doPcmp(thread_ctx, dst, expr_src1, expr_src2, kind, width);
}

void PIN_FAST_ANALYSIS_CALL
instrumentVinsert128iRegRegRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2,
    ADDRINT imm) {
  ExprRef expr_src1 = NULL;
  ExprRef expr_src2 = NULL;

  if (!getExprFromRegReg(
        thread_ctx,
        ctx,
        src1,
        src2,
        &expr_src1,
        &expr_src2)) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  ExprRef expr_res = doVinsert128i(expr_src1, expr_src2, imm);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentVinsert128iRegRegMemImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    MEM_ARG,
    ADDRINT imm) {
  ExprRef expr_src1 = NULL;
  ExprRef expr_src2 = NULL;

  if (!getExprFromRegMem(
        thread_ctx,
        ctx,
        src1,
        base, index, addr, size, disp, scale,
        &expr_src1,
        &expr_src2)) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  ExprRef expr_res = doVinsert128i(expr_src1, expr_src2, imm);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentVpmaddwdRegRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2) {
  ExprRef expr_src1 = NULL;
  ExprRef expr_src2 = NULL;

  if (!getExprFromRegReg(thread_ctx, ctx, src1, src2, &expr_src1, &expr_src2)) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  doPmaddwd(thread_ctx, dst, expr_src1, expr_src2);
}

void PIN_FAST_ANALYSIS_CALL
instrumentVpmaddwdRegRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    MEM_ARG) {
  ExprRef expr_src1 = NULL;
  ExprRef expr_src2 = NULL;

  if (!getExprFromRegMem(
        thread_ctx, ctx, src1,
        base, index, addr, size, disp, scale,
        &expr_src1, &expr_src2)) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  doPmaddwd(thread_ctx, dst, expr_src1, expr_src2);
}

void PIN_FAST_ANALYSIS_CALL
instrumentVpcmpxchgRegRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2,
    Kind kind,
    UINT width) {
  ExprRef expr_src1 = NULL;
  ExprRef expr_src2 = NULL;

  if (!getExprFromRegReg(thread_ctx, ctx, src1, src2, &expr_src1, &expr_src2)) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  doPcmpxchg(thread_ctx, dst, expr_src1, expr_src2, kind, width);
}

void PIN_FAST_ANALYSIS_CALL
instrumentVpcmpxchgRegRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    MEM_ARG,
    Kind kind,
    UINT width) {
  ExprRef expr_src1 = NULL;
  ExprRef expr_src2 = NULL;

  if (!getExprFromRegMem(
        thread_ctx, ctx, src1,
        base, index, addr, size, disp, scale,
        &expr_src1, &expr_src2)) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  doPcmpxchg(thread_ctx, dst, expr_src1, expr_src2, kind, width);
}

void PIN_FAST_ANALYSIS_CALL
instrumentVpmulhwRegRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2,
    bool sign) {
  ExprRef expr_src1 = NULL;
  ExprRef expr_src2 = NULL;

  if (!getExprFromRegReg(thread_ctx, ctx, src1, src2, &expr_src1, &expr_src2)) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  doPmulhw(thread_ctx, dst, expr_src1, expr_src2, sign);
}

void PIN_FAST_ANALYSIS_CALL
instrumentVpmulhwRegRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    MEM_ARG,
    bool sign) {
  ExprRef expr_src1 = NULL;
  ExprRef expr_src2 = NULL;

  if (!getExprFromRegMem(
        thread_ctx, ctx, src1,
        base, index, addr, size, disp, scale,
        &expr_src1, &expr_src2)) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  doPmulhw(thread_ctx, dst, expr_src1, expr_src2, sign);
}

void PIN_FAST_ANALYSIS_CALL
instrumentVpmulRegRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2,
    UINT width) {
  ExprRef expr_src1 = NULL;
  ExprRef expr_src2 = NULL;

  if (!getExprFromRegReg(thread_ctx, ctx, src1, src2, &expr_src1, &expr_src2)) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  doPmul(thread_ctx, dst, expr_src1, expr_src2, width);
}

void PIN_FAST_ANALYSIS_CALL
instrumentVpmulRegRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    MEM_ARG,
    UINT width) {
  ExprRef expr_src1 = NULL;
  ExprRef expr_src2 = NULL;

  if (!getExprFromRegMem(
        thread_ctx, ctx, src1,
        base, index, addr, size, disp, scale,
        &expr_src1, &expr_src2)) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  doPmul(thread_ctx, dst, expr_src1, expr_src2, width);
}

void PIN_FAST_ANALYSIS_CALL
instrumentVpshufbRegRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2) {
  ExprRef expr_src1 = NULL;
  ExprRef expr_src2 = NULL;

  if (!getExprFromRegReg(thread_ctx, ctx, src1, src2, &expr_src1, &expr_src2)) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  doPshufb(thread_ctx, dst, expr_src1, expr_src2);
}

void PIN_FAST_ANALYSIS_CALL
instrumentVpshufbRegRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    MEM_ARG) {
  ExprRef expr_src1 = NULL;
  ExprRef expr_src2 = NULL;

  if (!getExprFromRegMem(
        thread_ctx, ctx, src1,
        base, index, addr, size, disp, scale,
        &expr_src1, &expr_src2)) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  doPshufb(thread_ctx, dst, expr_src1, expr_src2);
}

static void
doPshufw(
    ThreadContext *thread_ctx,
    REG dst,
    ExprRef expr_src,
    ADDRINT imm,
    bool high) {
  UINT size = REG_Size(dst);
  UINT bits = size * CHAR_BIT;
  std::list<ExprRef> exprs;

  QSYM_ASSERT(size == 16 || size == 32);

  for (UINT j = 0; j < size / 16; j++) {
    UINT beg = j * 16 * CHAR_BIT;

    if (high)
      exprs.push_front(
          g_expr_builder->createExtract(expr_src, beg, 64)
      );

    for (UINT i = 0; i < 8; i+=2) {
      ExprRef expr_const = g_expr_builder->createConstant(
          ((imm >> i) & 3) * 16 + beg, bits);
      ExprRef expr_sub_res =
        g_expr_builder->createExtract(
            g_expr_builder->createLShr(expr_src, expr_const),
            high * 64, 16);
      exprs.push_front(expr_sub_res);
    }

    if (!high)
      exprs.push_front(
          g_expr_builder->createExtract(expr_src, beg + 64, 64)
      );
  }

  ExprRef expr_res = g_expr_builder->createConcat(exprs);
  thread_ctx->setExprToReg(dst, expr_res);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPshufwRegRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src,
    ADDRINT imm,
    bool high) {
  ExprRef expr_src = thread_ctx->getExprFromReg(ctx, src);
  if (expr_src == NULL) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  doPshufw(thread_ctx, dst, expr_src, imm, high);
}

void PIN_FAST_ANALYSIS_CALL
instrumentPshufwRegMemImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    MEM_ARG,
    ADDRINT imm,
    bool high) {
  ExprRef expr_src = getExprFromMemSrc(thread_ctx, ctx, base, index, addr, size,
                                       disp, scale);
  if (expr_src == NULL) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  doPshufw(thread_ctx, dst, expr_src, imm, high);
}

void PIN_FAST_ANALYSIS_CALL
instrumentVpshiftRegRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2,
    Kind kind,
    UINT width) {
  ExprRef expr_src1 = thread_ctx->getExprFromReg(ctx, src1);

  // TODO: support non-linear operation
  concretizeReg(thread_ctx, ctx, src2);

  if (expr_src1 == NULL) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  llvm::APInt value = getRegValue(ctx, src2);
  doPshift(thread_ctx, dst,
      kind, expr_src1, value.getLimitedValue(), width);
}

void PIN_FAST_ANALYSIS_CALL
instrumentVpshiftRegRegImm(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    ADDRINT imm,
    Kind kind,
    UINT width) {
  ExprRef expr_src1 = thread_ctx->getExprFromReg(ctx, src1);

  if (expr_src1 == NULL) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  doPshift(thread_ctx, dst, kind, expr_src1, imm, width);
}

void PIN_FAST_ANALYSIS_CALL
instrumentVpshiftRegRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    MEM_ARG,
    Kind kind,
    UINT width) {
  ExprRef expr_src1 = thread_ctx->getExprFromReg(ctx, src1);

  // TODO: support non-linear operation
  makeAddrConcrete(thread_ctx, ctx, base, index, addr, size, disp, scale);
  if (expr_src1 == NULL) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  llvm::APInt value = getMemValue(addr, size);
  doPshift(thread_ctx, dst, kind, expr_src1, value.getLimitedValue(), width);
}

void PIN_FAST_ANALYSIS_CALL
instrumentVpackRegRegReg(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    REG src2,
    UINT width,
    bool sign) {
  ExprRef expr_src1 = NULL;
  ExprRef expr_src2 = NULL;

  if (!getExprFromRegReg(thread_ctx, ctx, src1, src2, &expr_src1, &expr_src2)) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  doPack(thread_ctx, dst, expr_src1, expr_src2, width, sign);
}

void PIN_FAST_ANALYSIS_CALL
instrumentVpackRegRegMem(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    REG dst,
    REG src1,
    MEM_ARG,
    UINT width,
    bool sign) {
  ExprRef expr_src1 = NULL;
  ExprRef expr_src2 = NULL;

  if (!getExprFromRegMem(
        thread_ctx, ctx, src1,
        base, index, addr, size, disp, scale,
        &expr_src1, &expr_src2)) {
    thread_ctx->clearExprFromReg(dst);
    return;
  }

  doPack(thread_ctx, dst, expr_src1, expr_src2, width, sign);
}

void PIN_FAST_ANALYSIS_CALL
instrumentCallAfter(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    ADDRINT ret_addr) {
  g_call_stack_manager.visitCall(ret_addr);
}

void PIN_FAST_ANALYSIS_CALL
instrumentRet(
    ThreadContext* thread_ctx,
    const CONTEXT* ctx,
    ADDRINT branch_addr) {
  g_call_stack_manager.visitRet(branch_addr);
}

} // namespace qsym
