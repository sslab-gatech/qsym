#ifndef QSYM_TRACE_H_
#define QSYM_TRACE_H_

#include "expr.h"

namespace qsym {

void trace_getExprFromMem(ExprRef e, ADDRINT addr, INT32 size);
void trace_getExprFromReg(ExprRef e, const CONTEXT* ctx, REG r);
void trace_addJcc(ExprRef e, const CONTEXT* ctx, bool taken);
void trace_addValue(ExprRef e, llvm::APInt value);
}
#endif
