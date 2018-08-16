#include "memory.h"
#include "trace.h"
#include "thread_context.h"

namespace qsym {

static llvm::APInt evaluate(ExprRef e) {
  ExprRef ce = e->evaluate();
  switch (ce->kind()) {
    // Cannot handle Bool case, but is handled by bitToBool
    case Constant:
      return castAs<ConstantExpr>(ce)->value();
    default:
      UNREACHABLE();
      return llvm::APInt(1, 1);
  }
}

void trace_getExprFromReg(ExprRef e, const CONTEXT* ctx, REG r) {
  if (e == NULL)
    return;
  llvm::APInt calculated = evaluate(e);
  llvm::APInt real = getRegValue(ctx, r);
  if (calculated != real) {
    LOG_FATAL("Mismatch!"
          "\npc=" + hexstr(PIN_GetContextReg(ctx, REG_INST_PTR))
        + "\nexpr=" + e->toString()
        + "\nreg=" + REG_StringShort(r)
        + "\nreal=" + real.toString(16, false)
        + "\ncalculate=" + calculated.toString(16, false)
        + "\n");
  }
}

void trace_getExprFromMem(ExprRef e, ADDRINT addr, INT32 size) {
  if (e == NULL)
    return;
  llvm::APInt calculated = evaluate(e);
  llvm::APInt real = getMemValue(addr, size);
  if (calculated != real) {
    LOG_FATAL("Mismatch!"
          "\nexpr=" + e->toString()
        + "\naddr=" + hexstr(addr)
        + "\nreal=" + real.toString(16, false)
        + "\ncalculate=" + calculated.toString(16, false)
        + "\n");
  }
}

void trace_addJcc(ExprRef e, const CONTEXT* ctx, bool taken) {
  ExprRef bit = g_expr_builder->boolToBit(e, 1);
  llvm::APInt real(1, taken);
  llvm::APInt calculated = evaluate(bit);
  if (calculated != real) {
    LOG_FATAL("Mismatch!"
          "\npc=" + hexstr(PIN_GetContextReg(ctx, REG_INST_PTR))
        + "\nexpr=" + e->toString()
        + "\nreal=" + real.toString(16, false)
        + "\ncalculate=" + calculated.toString(16, false)
        + "\n");
  }
}

void trace_addValue(ExprRef e, llvm::APInt real) {
  llvm::APInt calculated = evaluate(e);
  if (calculated != real) {
    LOG_FATAL("Mismatch!"
          "\nexpr=" + e->toString()
        + "\nreal=" + real.toString(16, false)
        + "\ncalculate=" + calculated.toString(16, false)
        + "\n");
  }
}
} // namespace qsym
