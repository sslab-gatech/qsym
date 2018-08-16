#include "flags.h"

namespace qsym {

static INT32 kTimeStampCounter = 0;

// FlagOperation
FlagOperation::FlagOperation()
  : flags_(0),
    timestamp_(0),
    expr_result_(NULL),
    expr_dst_(NULL),
    expr_src_(NULL) {}

FlagOperation::~FlagOperation() {}

void FlagOperation::set(OpKind kind, ExprRef expr_result,
    ExprRef expr_dst, ExprRef expr_src) {
  kind_ = kind;
  flags_ = getEflagsFromOpKind(kind_);
  timestamp_ = kTimeStampCounter++;
  expr_result_ = expr_result;
  expr_src_ = expr_src;
  expr_dst_ = expr_dst;
}

ExprRef FlagOperation::computeCF() {
  // need to be implemented
  assert(0);
  return NULL;
}

ExprRef FlagOperation::computePF() {
  ExprRef expr_bit_zero = g_expr_builder->createConstant(0, 1);
  ExprRef expr_res = NULL;
  for (INT i = 0; i < 8; i++) {
    ExprRef expr_bit = g_expr_builder->createExtract(
          expr_result_, i, 1);
    if (expr_res == NULL)
      expr_res = expr_bit;
    else
      expr_res = g_expr_builder->createXor(expr_bit, expr_res);
  }
  ExprRef e = g_expr_builder->createEqual(expr_bit_zero, expr_res);
  return e;
}

ExprRef FlagOperation::computeOF() {
  // need to be implemented
  assert(0);
  return NULL;
}

ExprRef FlagOperation::computeSF() {
  ExprRef zero = g_expr_builder->createConstant(0, expr_result_->bits());
  return g_expr_builder->createBinaryExpr(Slt, expr_result_, zero);
}

ExprRef FlagOperation::computeZF() {
  ExprRef zero = g_expr_builder->createConstant(0, expr_result_->bits());
  return g_expr_builder->createBinaryExpr(Equal, expr_result_, zero);
}

ExprRef AddFlagOperation::computeCF() {
  return g_expr_builder->createBinaryExpr(Ult, expr_result(), expr_src());
}

ExprRef AddFlagOperation::computeOF() {
  // msb(not(src ^ dst)) & msb(res ^ dst)
  ExprRef e = g_expr_builder->createBinaryExpr(Xor, expr_dst(), expr_src());
  e = g_expr_builder->createNot(e);
  e = g_expr_builder->createMsb(e);

  ExprRef e2 = g_expr_builder->createBinaryExpr(Xor, expr_result(), expr_dst());
  e2 = g_expr_builder->createMsb(e2);

  e = g_expr_builder->createBinaryExpr(And, e, e2);
  return g_expr_builder->bitToBool(e);
}

ExprRef SubFlagOperation::computeCF() {
  return g_expr_builder->createBinaryExpr(Ult, expr_dst(), expr_src());
}

ExprRef SubFlagOperation::computeOF() {
  // sub = msb(src ^ dst) & msb(res ^ dst)
  ExprRef e = g_expr_builder->createBinaryExpr(Xor, expr_dst(), expr_src());
  e = g_expr_builder->createMsb(e);

  ExprRef e2 = g_expr_builder->createBinaryExpr(Xor, expr_result(), expr_dst());
  e2 = g_expr_builder->createMsb(e2);

  e = g_expr_builder->createBinaryExpr(And, e, e2);
  return g_expr_builder->bitToBool(e);
}

ExprRef LogicFlagOperation::computeCF() {
  return g_expr_builder->createFalse();
}

ExprRef LogicFlagOperation::computeOF() {
  return g_expr_builder->createFalse();
}

ExprRef IncFlagOperation::computeOF() {
  llvm::APInt v(expr_result()->bits(), 0);
  v++;
  v <<= (expr_result()->bits() - 1);
  ExprRef expr_imm = g_expr_builder->createConstant(v, expr_result()->bits());
  return g_expr_builder->createBinaryExpr(Equal, expr_result(), expr_imm);
}

ExprRef DecFlagOperation::computeOF() {
  llvm::APInt v(expr_result()->bits(), 0);
  v++;
  v <<= (expr_result()->bits() - 1);
  v--;
  ExprRef expr_imm = g_expr_builder->createConstant(v, expr_result()->bits());
  return g_expr_builder->createBinaryExpr(Equal, expr_result(), expr_imm);
}

ExprRef ShiftFlagOperation::computeOF() {
  ExprRef e = g_expr_builder->createBinaryExpr(Xor, expr_result(), expr_dst());
  e = g_expr_builder->createMsb(e);
  return g_expr_builder->bitToBool(e);
}

ExprRef ShlFlagOperation::computeCF() {
  return g_expr_builder->bitToBool(g_expr_builder->createMsb(expr_dst()));
}

ExprRef ShrFlagOperation::computeCF() {
  return g_expr_builder->bitToBool(g_expr_builder->createLsb(expr_dst()));
}

ExprRef RolFlagOperation::computeCF() {
  return g_expr_builder->bitToBool(g_expr_builder->createLsb(expr_result()));
}

ExprRef RolFlagOperation::computeOF() {
  // of = msb(result) ^ lsb(result)
  ExprRef e = g_expr_builder->createMsb(expr_result());
  ExprRef e2 = g_expr_builder->createLsb(expr_result());
  e = g_expr_builder->createBinaryExpr(Xor, e, e2);
  return g_expr_builder->bitToBool(e);
}

ExprRef RorFlagOperation::computeCF() {
  return g_expr_builder->bitToBool(g_expr_builder->createMsb(expr_result()));
}

ExprRef RorFlagOperation::computeOF() {
  // of' = msb(result) ^ msb-1(result).
  ExprRef e = g_expr_builder->createMsb(expr_result());
  ExprRef e2 = g_expr_builder->createExtract(expr_result(), expr_result()->bits() - 2, 1);
  e = g_expr_builder->createBinaryExpr(Xor, e, e2);
  return g_expr_builder->bitToBool(e);
}

ExprRef MulFlagOperation::_computeOF(bool sign) {
   ExprRef res = g_expr_builder->createExtract(expr_result(),
      0, expr_result()->bits() / 2);
  if (sign)
    res = g_expr_builder->createSExt(res, expr_result()->bits());
  else
    res = g_expr_builder->createZExt(res, expr_result()->bits());
  return g_expr_builder->createBinaryExpr(Equal, res, expr_result());
}

ExprRef SMulFlagOperation::computeCF() {
  // cf == of
  return computeOF();
}

ExprRef SMulFlagOperation::computeOF() {
  return _computeOF(true);
}

ExprRef UMulFlagOperation::computeCF() {
  // cf == of
  return computeOF();
}

ExprRef UMulFlagOperation::computeOF() {
  return _computeOF(false);
}

ExprRef BtFlagOperation::computeCF() {
  // save CF value in expr_result_
  return expr_result_;
}

// Eflags
Eflags::Eflags()
  : valid_set_(0),
    operations_(),
    start_(0) {
  memset(op_kinds_, -1, sizeof(op_kinds_));

  // set operations by the value
  operations_[CC_OP_ADD] = new AddFlagOperation();
  operations_[CC_OP_SUB] = new SubFlagOperation();
  operations_[CC_OP_LOGIC] = new LogicFlagOperation();
  operations_[CC_OP_INC] = new IncFlagOperation();
  operations_[CC_OP_DEC] = new DecFlagOperation();
  operations_[CC_OP_SHL] = new ShlFlagOperation();
  operations_[CC_OP_SHR] = new ShrFlagOperation();
  operations_[CC_OP_ROR] = new RorFlagOperation();
  operations_[CC_OP_ROL] = new RolFlagOperation();
  operations_[CC_OP_SMUL] = new SMulFlagOperation();
  operations_[CC_OP_UMUL] = new UMulFlagOperation();
  operations_[CC_OP_BT] = new BtFlagOperation();
}

Eflags::~Eflags() {
  for (INT i = 0; i < (INT)CC_OP_LAST; i++) {
    delete operations_[i];
  }
}

void Eflags::set(OpKind kind, ExprRef expr_result,
      ExprRef expr_dst, ExprRef expr_src) {
    UINT32 next_start = (start_ + 1) % CC_OP_LAST;
    validate(kind);

    // record the flag operation & save op_kind for ordering
    operations_[kind]->set(kind, expr_result, expr_dst, expr_src);
    op_kinds_[next_start] = kind;
    start_ = next_start;
}

ExprRef Eflags::computeJcc(const CONTEXT* ctx, JccKind jcc_c, bool inv) {
  if (!isValid(jcc_c))
    return NULL;
  ExprRef e = computeFastJcc(ctx, jcc_c, inv);
  if (e == NULL)
    e = computeSlowJcc(ctx, jcc_c, inv);
  if (e == NULL) {
    LOG_INFO("unhandled operation: jcc=" + std::to_string(jcc_c)
        + ", inv=" + std::to_string(inv) + "\n");
  }

  return e;
}

ExprRef Eflags::computeJccAsBV(const CONTEXT* ctx, JccKind jcc_c, bool inv, INT32 size) {
  ExprRef e = computeJcc(ctx, jcc_c, inv);
  if (e)
    return g_expr_builder->boolToBit(e, size);
  else
    return NULL;
}

ExprRef Eflags::computeFastJcc(const CONTEXT* ctx,
    JccKind jcc_c, bool inv) {
  // check if last operation is CC_OP_SUB
  OpKind kind = op_kinds_[start_];
  FlagOperation* flag_op = operations_[kind];
  UINT32 flags = getEflagsFromOpKind(kind);
  Kind op;

  if (kind == CC_OP_SUB) {
    switch (jcc_c) {
      case JCC_B:
        op = Ult;
        break;
      case JCC_BE:
        op = Ule;
        break;
      case JCC_L:
        op = Slt;
        break;
      case JCC_LE:
        op = Sle;
        break;
      case JCC_Z:
        op = Equal;
        break;
      case JCC_S:
        goto fast_jcc_s;
      default:
        return NULL;
    }

    if (inv)
      op = negateKind(op);
    ExprRef e = g_expr_builder->createBinaryExpr(op, flag_op->expr_dst(), flag_op->expr_src());
    return e;
  }

fast_jcc_s:
    // if operation is affected on ZF and SF
    if ((flags & EFLAGS_SF)
        && jcc_c == JCC_S) {
        if (inv)
          op = Sge;
        else
          op = Slt;
    }
    else if ((flags & EFLAGS_ZF)
              && jcc_c == JCC_Z) {
        if (inv)
          op = Distinct;
        else
          op = Equal;
    }
    else {
      // default case
      return NULL;
    }

    ExprRef expr_zero = g_expr_builder->createConstant(0, flag_op->expr_result()->bits());
    return g_expr_builder->createBinaryExpr(op, flag_op->expr_result(), expr_zero);
}

ExprRef Eflags::computeSlowJcc(const CONTEXT* ctx, JccKind jcc_c, bool inv) {
  ExprRef e = NULL;
  bool equal = false;

  switch (jcc_c) {
    case JCC_BE:
      equal = true;
    case JCC_B:
      e = computeFlag(EFLAGS_CF);
      break;
    case JCC_LE:
      equal = true;
    case JCC_L:
      e = computeFlag(EFLAGS_SF);
      e = g_expr_builder->createDistinct(e, computeFlag(EFLAGS_OF));
      break;
    case JCC_S:
      e = computeFlag(EFLAGS_SF);
      break;
    case JCC_Z:
      e = computeFlag(EFLAGS_ZF);
    case JCC_O:
      e = computeFlag(EFLAGS_OF);
      break;
    case JCC_P:
      e = computeFlag(EFLAGS_PF);
      break;
    default:
      UNREACHABLE();
      return NULL;
  }

  if (equal)
    e = g_expr_builder->createLOr(e, computeFlag(EFLAGS_ZF));

  if (inv)
    e = g_expr_builder->createLNot(e);

  return e;
}

ExprRef Eflags::computeFlag(Eflag flag) {
  for (INT i = 0; i < CC_OP_LAST; i++) {
    UINT32 start = (start_ - i) % CC_OP_LAST;
    OpKind op_kind = op_kinds_[start];
    FlagOperation *flag_op = operations_[op_kind];

    if ((flag_op->flags() & flag) != 0) {
      // the flag is generated by this operation

      switch (flag) {
        case EFLAGS_CF:
          return flag_op->computeCF();
        case EFLAGS_PF:
          return flag_op->computePF();
        case EFLAGS_ZF:
          return flag_op->computeZF();
        case EFLAGS_SF:
          return flag_op->computeSF();
        case EFLAGS_OF:
          return flag_op->computeOF();
        default:
          return NULL;
      }
    }
  }

  // we could not find the operation for the flag
  return NULL;
}

} // namespace qsym
