#include "thread_context.h"

namespace qsym {

const INT32 kLimitDepth = 32;

namespace {

inline ExprRef getBaseExpr(
    const CONTEXT* ctx, REG base, ADDRINT disp, ExprRef e) {
  // memory address = base + disp + index * scale
  if (e != NULL) {
    if (disp != 0) {
      ExprRef expr_disp = g_expr_builder->createConstant(disp, e->bits());
      e = g_expr_builder->createBinaryExpr(Add, e, expr_disp);
    }
    // else use e
  }
  else if (base != REG_INVALID()) {
    ADDRINT val_base = getRegValue(ctx, base).getZExtValue();
    e = g_expr_builder->createConstant(
        val_base + disp, sizeof(ADDRINT) * 8);
  }
  else if (disp != 0)
    e = g_expr_builder->createConstant(disp, sizeof(ADDRINT) * 8);
  return e;
}

inline ExprRef getIndexExpr(
  const CONTEXT* ctx, REG index, UINT32 scale, ExprRef e) {

  if (e != NULL) {
    if (scale != 1) {
      ExprRef expr_scale = g_expr_builder->createConstant(scale, e->bits());
      e = g_expr_builder->createBinaryExpr(Mul, e, expr_scale);
    }
    // else use e
  }
  else if (index != REG_INVALID()) {
    ADDRINT val_index = getRegValue(ctx, index).getZExtValue();
    e = g_expr_builder->createConstant(val_index * scale,
        sizeof(ADDRINT) * 8);
  }
  return e;
}

inline ExprRef createAddrExpr(ExprRef expr_base, ExprRef expr_index) {
  if (expr_base && expr_index)
      return g_expr_builder->createAdd(expr_base, expr_index);
  else if (expr_index)
      return expr_index;
  else if (expr_base)
    return expr_base;
  else {
    UNREACHABLE();
    return NULL;
  }
}

} // namespace

void ThreadContext::onSyscallEnter(CONTEXT* ctx, SYSCALL_STANDARD std) {
  size_t syscall_nr = PIN_GetSyscallNumber(ctx, std);
  if (syscall_nr >= kSyscallMax) {
    LOG_INFO("unknown syscall (num=" + decstr(syscall_nr) + ")\n");
    syscall_ctx_.nr = -1;
    return;
  }

  SyscallDesc& syscall_desc = kSyscallDesc[syscall_nr];
  syscall_ctx_.nr = syscall_nr;

  if (syscall_desc.save_args
        | syscall_desc.retval_args) {
    for (USIZE i = 0 ; i < syscall_desc.nargs; i++)
      syscall_ctx_.arg[i] = PIN_GetSyscallArgument(ctx, std, i);
    syscall_ctx_.aux = ctx;
    if (syscall_desc.pre != NULL)
      syscall_desc.pre(&syscall_ctx_);
  }
}

void ThreadContext::onSyscallExit(CONTEXT* ctx, SYSCALL_STANDARD std) {
  size_t syscall_nr = syscall_ctx_.nr;
  if (syscall_nr >= kSyscallMax) {
    LOG_INFO("unknown syscall (num=" + decstr(syscall_nr) + ")\n");
    return;
  }

  SyscallDesc& syscall_desc = kSyscallDesc[syscall_nr];

  if (syscall_desc.save_args
      | syscall_desc.retval_args) {
    syscall_ctx_.ret = PIN_GetSyscallReturn(ctx, std);
    syscall_ctx_.aux = ctx;

    if (syscall_desc.post != NULL)
      syscall_desc.post(&syscall_ctx_);
    else {
      if (syscall_ctx_.ret < 0)
        return;

      clearArgs(syscall_desc);
    }
  }
}

void ThreadContext::clearArgs(SyscallDesc& syscall_desc) {
  for (USIZE i = 0; i < syscall_desc.nargs; i++) {
    if (syscall_desc.map_args[i] > 0 &&
        (void*)syscall_ctx_.arg[i] != NULL)
      g_memory.clearExprFromMem(syscall_ctx_.arg[i],
          syscall_desc.map_args[i]);
  }
}

ExprRef ThreadContext::getAddrExpr(const CONTEXT* ctx,
    REG base, REG index, ADDRINT disp, UINT32 scale) {
  ExprRef expr_base = NULL;
  ExprRef expr_index = NULL;
  UINT32 bits = sizeof(ADDRINT) * CHAR_BIT;

  if (base != REG_INVALID())
    expr_base = getExprFromReg(ctx, base);
  if (index != REG_INVALID())
    expr_index = getExprFromReg(ctx, index);

  if (expr_base == NULL
      && expr_index == NULL)
    return NULL;

  // extend to `bits`-size
  if (expr_base != NULL) {
    QSYM_ASSERT(expr_base->bits() <= bits);
    expr_base = g_expr_builder->createZExt(expr_base, bits);
  }

  if (expr_index != NULL) {
    QSYM_ASSERT(expr_index->bits() <= bits);
    expr_index = g_expr_builder->createZExt(expr_index, bits);
  }

  expr_base = getBaseExpr(ctx, base, disp, expr_base);
  expr_index = getIndexExpr(ctx, index, scale, expr_index);

  return createAddrExpr(expr_base, expr_index);
}

} // namespace qsym
