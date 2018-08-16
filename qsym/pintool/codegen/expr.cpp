#include "expr_builder.h"
#include "expr.h"
#include "solver.h"

// NOTE: Some simplification is ported from KLEE
namespace qsym {

KNOB<bool> g_opt_debug_subsumption(KNOB_MODE_WRITEONCE, "pintool",
    "debug_subsumption", "0", "debug constraints subsumption");

Kind swapKind(Kind kind) {
  // function for finding neg_op s.t. x op y ==> y neg_op x
  switch (kind) {
    case Equal:
      return Equal;
    case Distinct:
      return Distinct;
    case Ult:
      return Ugt;
    case Ule:
      return Uge;
    case Ugt:
      return Ult;
    case Uge:
      return Ule;
    case Slt:
      return Sgt;
    case Sle:
      return Sge;
    case Sgt:
      return Slt;
    case Sge:
      return Sle;
    default:
      UNREACHABLE();
      return Invalid;
  }
}

Kind negateKind(Kind kind) {
  // function for finding neg_op s.t. x op y ==> y neg_op x
  switch (kind) {
    case Equal:
      return Distinct;
    case Distinct:
      return Equal;
    case Ult:
      return Uge;
    case Ule:
      return Ugt;
    case Ugt:
      return Ule;
    case Uge:
      return Ult;
    case Slt:
      return Sge;
    case Sle:
      return Sgt;
    case Sgt:
      return Sle;
    case Sge:
      return Slt;
    default:
      UNREACHABLE();
      return Invalid;
  }
}

bool isNegatableKind(Kind kind) {
  switch (kind) {
    case Distinct:
    case Equal:
    case Ult:
    case Ule:
    case Ugt:
    case Uge:
    case Slt:
    case Sle:
    case Sgt:
    case Sge:
      return true;
    default:
      return false;
  }
}

bool isZeroBit(ExprRef e, UINT32 idx) {
  if (ConstantExprRef ce = castAs<ConstantExpr>(e))
    return !ce->value()[idx];
  if (auto ce = castAs<ConcatExpr>(e)) {
    if (ce->getRight()->bits() <= idx)
      return isZeroBit(ce->getLeft(), idx - ce->getRight()->bits());
    else
      return isZeroBit(ce->getRight(), idx);
  }
  return false; // otherwise return false
}

bool isOneBit(ExprRef e, UINT32 idx) {
  if (ConstantExprRef ce = castAs<ConstantExpr>(e))
    return ce->value()[idx];
  if (auto ce = castAs<ConcatExpr>(e)) {
    if (ce->getRight()->bits() <= idx)
      return isOneBit(ce->getLeft(), idx - ce->getRight()->bits());
    else
      return isOneBit(ce->getRight(), idx);
  }
  return false; // otherwise return false
}

bool isRelational(const Expr* e) {
  switch (e->kind()) {
    case Distinct:
    case Equal:
    case Ult:
    case Ule:
    case Ugt:
    case Uge:
    case Slt:
    case Sle:
    case Sgt:
    case Sge:
    case LAnd:
    case LOr:
    case LNot:
      return true;
    default:
      return false;
  }
}

bool isConstant(ExprRef e) {
 return e->kind() == Constant;
}

bool isConstSym(ExprRef e) {
  if (e->num_children() != 2)
    return false;
  for (UINT32 i = 0; i < 2; i++) {
    if (isConstant(e->getChild(i))
        && !isConstant(e->getChild(1 - i)))
      return true;
  }
  return false;
}

UINT32 getMSB(
    ExprRef e)
{
  UINT32 i = 0;
  assert(e->bits() >= 1);
  for (i = e->bits() - 1; i-- > 0;) {
    if (!isZeroBit(e, i))
      break;
  }

  return i + 1;
}

// Expr declaration
Expr::Expr(Kind kind, UINT32 bits)
  : DependencyNode()
  , kind_(kind)
  , bits_(bits)
  , children_()
  , context_(g_z3_context)
  , expr_(NULL)
  , hash_(NULL)
  , range_sets{}
  , isConcrete_(true)
  , depth_(-1)
  , deps_(NULL)
  , leading_zeros_((UINT)-1)
  , evaluation_(NULL)
  {}

Expr::~Expr() {
  delete expr_;
  delete hash_;
  delete range_sets[0];
  delete range_sets[1];
  delete deps_;
}

DependencySet Expr::computeDependencies() {
  DependencySet deps;
  for (const int& dep : getDeps())
    deps.insert((size_t)dep);
  return deps;
}

XXH32_hash_t Expr::hash() {
  if (hash_ == NULL) {
    XXH32_state_t state;
    XXH32_reset(&state, 0); // seed = 0
    XXH32_update(&state, &kind_, sizeof(kind_));
    XXH32_update(&state, &bits_, sizeof(bits_));
    for (INT32 i = 0; i < num_children(); i++) {
      XXH32_hash_t h = children_[i]->hash();
      XXH32_update(&state, &h, sizeof(h));
    }
    hashAux(&state);
    hash_ = new XXH32_hash_t(XXH32_digest(&state));
  }
  return *hash_;
}

INT32 Expr::depth() {
  if (depth_ == -1) {
    INT32 max_depth = 0;
    for (INT32 i = 0; i < num_children(); i++) {
      INT32 child_depth = getChild(i)->depth();
      if (child_depth > max_depth)
        max_depth = child_depth;
    }
    depth_ = max_depth + 1;
  }
  return depth_;
}


void Expr::print(ostream& os, UINT depth) const {
  os << getName() << "(";
  bool begin = !printAux(os);
  printChildren(os, begin, depth);
  os << ")";
}

void Expr::printConstraints() {
  for (UINT32 i = 0; i < 2; i++) {
    RangeSet *rs = getRangeSet(i);
    if (rs) {
      std::cerr << "\t " << (i ? "unsigned" : "signed") << ":";
      rs->print(std::cerr);
      std::cerr << "\n";
    }
  }
}

std::string Expr::toString() const {
  std::ostringstream stream;
  this->print(stream);
  return stream.str();
}

void Expr::simplify() {
      if (isRelational(this)) {
        // If an expression is relational expression,
        // then simplify children to reuse the simplified expressions
        for (INT32 i = 0; i < num_children(); i++)
          children_[i]->simplify();
      }
      else {
        if (expr_ == NULL) {
          z3::expr z3_expr = toZ3Expr(true);
          z3_expr = z3_expr.simplify();
          expr_ = new z3::expr(z3_expr);
        }
      }
}

void Expr::printChildren(ostream& os, bool start, UINT depth) const {
  for (INT32 i = 0; i < num_children(); i++) {
    if (start)
      start = false;
    else
      os << ", ";
    children_[i]->print(os, depth + 1);
  }
}

bool Expr::isZero() const {
  if (isConstant()) {
    const ConstantExpr* me = static_cast<const ConstantExpr*>(this);
    return me->isZero();
  }
  else
    return false;
}

bool Expr::isAllOnes() const {
  if (isConstant()) {
    const ConstantExpr* me = static_cast<const ConstantExpr*>(this);
    return me->isAllOnes();
  }
  else
    return false;
}

bool Expr::isOne() const {
  if (isConstant()) {
    const ConstantExpr* me = static_cast<const ConstantExpr*>(this);
    return me->isOne();
  }
  else
    return false;
}

void Expr::addConstraint(
    bool is_unsigned,
    Kind kind,
    llvm::APInt& rhs,
    llvm::APInt& adjustment) {
  RangeSet *rs = getRangeSet(is_unsigned);
  if (rs == NULL) {
    rs = new RangeSet(is_unsigned, bits());
    setRangeSet(is_unsigned, rs);
  }
  addConstraint(*rs, kind, rhs, adjustment);
}

void Expr::addConstraint(
    RangeSet& range_set,
    Kind kind,
    llvm::APInt& rhs,
    llvm::APInt& adjustment) {
  // C + A rel V
  // C - A rel V ==> A + V swap(rel) C
  switch (kind) {
    case Slt:
    case Ult:
      range_set = range_set.intersectLT(rhs, adjustment);
      break;
    case Sle:
    case Ule:
      range_set = range_set.intersectLE(rhs, adjustment);
      break;
    case Sgt:
    case Ugt:
      range_set = range_set.intersectGT(rhs, adjustment);
      break;
    case Sge:
    case Uge:
      range_set = range_set.intersectGE(rhs, adjustment);
      break;
    case Equal:
      range_set = range_set.intersectEQ(rhs, adjustment);
      break;
    case Distinct:
      range_set = range_set.intersectNE(rhs, adjustment);
      break;
    default:
      UNREACHABLE();
      break;
  }
}

void Expr::addConstraint(Kind kind, llvm::APInt rhs, llvm::APInt adjustment) {
  if (g_opt_debug_subsumption.Value()) {
    LOG_INFO("Before subsumption: " + this->toString() + "\n");
    printConstraints();
  }

  switch (kind) {
    case Slt:
    case Sle:
    case Sgt:
    case Sge:
      addConstraint(false, kind, rhs, adjustment);
      break;
    case Ult:
    case Ule:
    case Ugt:
    case Uge:
      addConstraint(true, kind, rhs, adjustment);
      break;
    case Equal:
    case Distinct:
      // only to the signed, it will be fine
      addConstraint(false, kind, rhs, adjustment);
      break;
    default:
      break;
  }

  if (g_opt_debug_subsumption.Value()) {
    LOG_INFO("After subsumption: " + this->toString() + "\n");
    printConstraints();
  }
}

void ConstantExpr::print(ostream& os, UINT depth) const {
    os << "0x" << value_.toString(16, false);
}

void ConcatExpr::print(ostream& os, UINT depth) const {
  bool start = true;
  for (INT32 i = 0; i < num_children(); i++) {
    if (start)
      start = false;
    else
      os << " | ";
    children_[i]->print(os, depth + 1);
  }
}

void BinaryExpr::print(ostream& os, UINT depth, const char* op) const {
  ExprRef c0 = getChild(0);
  ExprRef c1 = getChild(1);
  bool c0_const = c0->isConstant();
  bool c1_const = c1->isConstant();

  if (!c0_const)
    os << "(";
  c0->print(os, depth + 1);
  if (!c0_const)
    os << ")";

  os << " " << op << " ";

  if (!c1_const)
    os << "(";
  c1->print(os, depth + 1);
  if (!c1_const)
    os << ")";
}

{CODEGEN}

} // namespace qsym

