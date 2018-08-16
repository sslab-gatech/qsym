#ifndef QSYM_EXPR_BUILDER_H_
#define QSYM_EXPR_BUILDER_H_

#include <list>

#include "expr_cache.h"

namespace qsym {

class ExprBuilder {
public:
  ExprBuilder();
  void setNext(ExprBuilder* next);

  // {BEGIN:FUNC}
  virtual ExprRef createBool(bool b);
  virtual ExprRef createConstant(ADDRINT value, UINT32 bits);
  virtual ExprRef createConstant(llvm::APInt value, UINT32 bits);
  virtual ExprRef createRead(ADDRINT off);
  virtual ExprRef createConcat(ExprRef l, ExprRef r);
  virtual ExprRef createExtract(ExprRef e, UINT32 index, UINT32 bits);
  virtual ExprRef createZExt(ExprRef e, UINT32 bits);
  virtual ExprRef createSExt(ExprRef e, UINT32 bits);
  virtual ExprRef createAdd(ExprRef l, ExprRef r);
  virtual ExprRef createSub(ExprRef l, ExprRef r);
  virtual ExprRef createMul(ExprRef l, ExprRef r);
  virtual ExprRef createUDiv(ExprRef l, ExprRef r);
  virtual ExprRef createSDiv(ExprRef l, ExprRef r);
  virtual ExprRef createURem(ExprRef l, ExprRef r);
  virtual ExprRef createSRem(ExprRef l, ExprRef r);
  virtual ExprRef createNeg(ExprRef e);
  virtual ExprRef createNot(ExprRef e);
  virtual ExprRef createAnd(ExprRef l, ExprRef r);
  virtual ExprRef createOr(ExprRef l, ExprRef r);
  virtual ExprRef createXor(ExprRef l, ExprRef r);
  virtual ExprRef createShl(ExprRef l, ExprRef r);
  virtual ExprRef createLShr(ExprRef l, ExprRef r);
  virtual ExprRef createAShr(ExprRef l, ExprRef r);
  virtual ExprRef createEqual(ExprRef l, ExprRef r);
  virtual ExprRef createDistinct(ExprRef l, ExprRef r);
  virtual ExprRef createUlt(ExprRef l, ExprRef r);
  virtual ExprRef createUle(ExprRef l, ExprRef r);
  virtual ExprRef createUgt(ExprRef l, ExprRef r);
  virtual ExprRef createUge(ExprRef l, ExprRef r);
  virtual ExprRef createSlt(ExprRef l, ExprRef r);
  virtual ExprRef createSle(ExprRef l, ExprRef r);
  virtual ExprRef createSgt(ExprRef l, ExprRef r);
  virtual ExprRef createSge(ExprRef l, ExprRef r);
  virtual ExprRef createLOr(ExprRef l, ExprRef r);
  virtual ExprRef createLAnd(ExprRef l, ExprRef r);
  virtual ExprRef createLNot(ExprRef e);
  virtual ExprRef createIte(ExprRef expr_cond, ExprRef expr_true, ExprRef expr_false);
  // {END:FUNC}

  // utility functions
  ExprRef createTrue();
  ExprRef createFalse();
  ExprRef createMsb(ExprRef);
  ExprRef createLsb(ExprRef);

  ExprRef bitToBool(ExprRef e);
  ExprRef boolToBit(ExprRef e, UINT32 bits);
  ExprRef createBinaryExpr(Kind kind, ExprRef l, ExprRef r);
  ExprRef createUnaryExpr(Kind kind, ExprRef e);
  ExprRef createConcat(std::list<ExprRef> exprs);
  ExprRef createLAnd(std::list<ExprRef> exprs);
  ExprRef createTrunc(ExprRef e, UINT32 bits);

protected:
  ExprBuilder* next_;
};

class BaseExprBuilder : public ExprBuilder {
public:
  ExprRef createExtract(ExprRef e, UINT32 index, UINT32 bits) override;
  ExprRef createRead(ADDRINT off) override;

  // {BEGIN:BASE}
  ExprRef createBool(bool b) override;
  ExprRef createConstant(ADDRINT value, UINT32 bits) override;
  ExprRef createConstant(llvm::APInt value, UINT32 bits) override;
  ExprRef createConcat(ExprRef l, ExprRef r) override;
  ExprRef createZExt(ExprRef e, UINT32 bits) override;
  ExprRef createSExt(ExprRef e, UINT32 bits) override;
  ExprRef createAdd(ExprRef l, ExprRef r) override;
  ExprRef createSub(ExprRef l, ExprRef r) override;
  ExprRef createMul(ExprRef l, ExprRef r) override;
  ExprRef createUDiv(ExprRef l, ExprRef r) override;
  ExprRef createSDiv(ExprRef l, ExprRef r) override;
  ExprRef createURem(ExprRef l, ExprRef r) override;
  ExprRef createSRem(ExprRef l, ExprRef r) override;
  ExprRef createNeg(ExprRef e) override;
  ExprRef createNot(ExprRef e) override;
  ExprRef createAnd(ExprRef l, ExprRef r) override;
  ExprRef createOr(ExprRef l, ExprRef r) override;
  ExprRef createXor(ExprRef l, ExprRef r) override;
  ExprRef createShl(ExprRef l, ExprRef r) override;
  ExprRef createLShr(ExprRef l, ExprRef r) override;
  ExprRef createAShr(ExprRef l, ExprRef r) override;
  ExprRef createEqual(ExprRef l, ExprRef r) override;
  ExprRef createDistinct(ExprRef l, ExprRef r) override;
  ExprRef createUlt(ExprRef l, ExprRef r) override;
  ExprRef createUle(ExprRef l, ExprRef r) override;
  ExprRef createUgt(ExprRef l, ExprRef r) override;
  ExprRef createUge(ExprRef l, ExprRef r) override;
  ExprRef createSlt(ExprRef l, ExprRef r) override;
  ExprRef createSle(ExprRef l, ExprRef r) override;
  ExprRef createSgt(ExprRef l, ExprRef r) override;
  ExprRef createSge(ExprRef l, ExprRef r) override;
  ExprRef createLOr(ExprRef l, ExprRef r) override;
  ExprRef createLAnd(ExprRef l, ExprRef r) override;
  ExprRef createLNot(ExprRef e) override;
  ExprRef createIte(ExprRef expr_cond, ExprRef expr_true, ExprRef expr_false) override;
  // {END:BASE}
};

class CacheExprBuilder : public ExprBuilder {
public:
  // {BEGIN:CACHE}
  ExprRef createConcat(ExprRef l, ExprRef r) override;
  ExprRef createExtract(ExprRef e, UINT32 index, UINT32 bits) override;
  ExprRef createZExt(ExprRef e, UINT32 bits) override;
  ExprRef createSExt(ExprRef e, UINT32 bits) override;
  ExprRef createAdd(ExprRef l, ExprRef r) override;
  ExprRef createSub(ExprRef l, ExprRef r) override;
  ExprRef createMul(ExprRef l, ExprRef r) override;
  ExprRef createUDiv(ExprRef l, ExprRef r) override;
  ExprRef createSDiv(ExprRef l, ExprRef r) override;
  ExprRef createURem(ExprRef l, ExprRef r) override;
  ExprRef createSRem(ExprRef l, ExprRef r) override;
  ExprRef createNeg(ExprRef e) override;
  ExprRef createNot(ExprRef e) override;
  ExprRef createAnd(ExprRef l, ExprRef r) override;
  ExprRef createOr(ExprRef l, ExprRef r) override;
  ExprRef createXor(ExprRef l, ExprRef r) override;
  ExprRef createShl(ExprRef l, ExprRef r) override;
  ExprRef createLShr(ExprRef l, ExprRef r) override;
  ExprRef createAShr(ExprRef l, ExprRef r) override;
  ExprRef createEqual(ExprRef l, ExprRef r) override;
  ExprRef createDistinct(ExprRef l, ExprRef r) override;
  ExprRef createUlt(ExprRef l, ExprRef r) override;
  ExprRef createUle(ExprRef l, ExprRef r) override;
  ExprRef createUgt(ExprRef l, ExprRef r) override;
  ExprRef createUge(ExprRef l, ExprRef r) override;
  ExprRef createSlt(ExprRef l, ExprRef r) override;
  ExprRef createSle(ExprRef l, ExprRef r) override;
  ExprRef createSgt(ExprRef l, ExprRef r) override;
  ExprRef createSge(ExprRef l, ExprRef r) override;
  ExprRef createLOr(ExprRef l, ExprRef r) override;
  ExprRef createLAnd(ExprRef l, ExprRef r) override;
  ExprRef createLNot(ExprRef e) override;
  ExprRef createIte(ExprRef expr_cond, ExprRef expr_true, ExprRef expr_false) override;
  // {END:CACHE}

protected:
  ExprCache cache_;

  void insertToCache(ExprRef e);
  ExprRef findInCache(ExprRef e);
  ExprRef findOrInsert(ExprRef new_expr);

};

class CommutativeExprBuilder : public ExprBuilder {
public:
  // {BEGIN:COMMUTATIVE}
  ExprRef createAdd(ExprRef l, ExprRef r) override;
  ExprRef createMul(ExprRef l, ExprRef r) override;
  ExprRef createAnd(ExprRef l, ExprRef r) override;
  ExprRef createOr(ExprRef l, ExprRef r) override;
  ExprRef createXor(ExprRef l, ExprRef r) override;
  ExprRef createEqual(ExprRef l, ExprRef r) override;
  ExprRef createDistinct(ExprRef l, ExprRef r) override;
  ExprRef createUlt(ExprRef l, ExprRef r) override;
  ExprRef createUle(ExprRef l, ExprRef r) override;
  ExprRef createUgt(ExprRef l, ExprRef r) override;
  ExprRef createUge(ExprRef l, ExprRef r) override;
  ExprRef createSlt(ExprRef l, ExprRef r) override;
  ExprRef createSle(ExprRef l, ExprRef r) override;
  ExprRef createSgt(ExprRef l, ExprRef r) override;
  ExprRef createSge(ExprRef l, ExprRef r) override;
  ExprRef createLAnd(ExprRef l, ExprRef r) override;
  ExprRef createLOr(ExprRef l, ExprRef r) override;
  // {END:COMMUTATIVE}
  ExprRef createSub(ExprRef l, ExprRef r) override;
};

class CommonSimplifyExprBuilder : public ExprBuilder {
  // expression builder for common simplification
public:
  ExprRef createConcat(ExprRef l, ExprRef r) override;
  ExprRef createExtract(ExprRef e, UINT32 index, UINT32 bits) override;
  ExprRef createZExt(ExprRef e, UINT32 bits) override;
  ExprRef createAdd(ExprRef l, ExprRef r) override;
  ExprRef createMul(ExprRef, ExprRef) override;
  ExprRef createAnd(ExprRef, ExprRef) override;
  ExprRef createOr(ExprRef, ExprRef) override;
  ExprRef createXor(ExprRef, ExprRef) override;
  ExprRef createShl(ExprRef l, ExprRef r) override;
  ExprRef createLShr(ExprRef l, ExprRef r) override;
  ExprRef createAShr(ExprRef l, ExprRef r) override;
  ExprRef createEqual(ExprRef l, ExprRef r) override;

private:
  ExprRef simplifyAnd(ExprRef l, ExprRef r);
  ExprRef simplifyOr(ExprRef l, ExprRef r);
  ExprRef simplifyXor(ExprRef l, ExprRef r);
};

class ConstantFoldingExprBuilder : public ExprBuilder {
public:
  ExprRef createConcat(ExprRef l, ExprRef r) override;
  ExprRef createExtract(ExprRef e, UINT32 index, UINT32 bits) override;
  ExprRef createZExt(ExprRef e, UINT32 bits) override;
  ExprRef createSExt(ExprRef e, UINT32 bits) override;
  ExprRef createAdd(ExprRef l, ExprRef r) override;
  ExprRef createSub(ExprRef l, ExprRef r) override;
  ExprRef createMul(ExprRef l, ExprRef r) override;
  ExprRef createUDiv(ExprRef l, ExprRef r) override;
  ExprRef createSDiv(ExprRef l, ExprRef r) override;
  ExprRef createURem(ExprRef l, ExprRef r) override;
  ExprRef createSRem(ExprRef l, ExprRef r) override;
  ExprRef createNeg(ExprRef e) override;
  ExprRef createNot(ExprRef e) override;
  ExprRef createAnd(ExprRef l, ExprRef r) override;
  ExprRef createOr(ExprRef l, ExprRef r) override;
  ExprRef createXor(ExprRef l, ExprRef r) override;
  ExprRef createShl(ExprRef l, ExprRef r) override;
  ExprRef createLShr(ExprRef l, ExprRef r) override;
  ExprRef createAShr(ExprRef l, ExprRef r) override;
  ExprRef createEqual(ExprRef l, ExprRef r) override;
  ExprRef createDistinct(ExprRef l, ExprRef r) override;
  ExprRef createUlt(ExprRef l, ExprRef r) override;
  ExprRef createUle(ExprRef l, ExprRef r) override;
  ExprRef createUgt(ExprRef l, ExprRef r) override;
  ExprRef createUge(ExprRef l, ExprRef r) override;
  ExprRef createSlt(ExprRef l, ExprRef r) override;
  ExprRef createSle(ExprRef l, ExprRef r) override;
  ExprRef createSgt(ExprRef l, ExprRef r) override;
  ExprRef createSge(ExprRef l, ExprRef r) override;
  ExprRef createLOr(ExprRef l, ExprRef r) override;
  ExprRef createLAnd(ExprRef l, ExprRef r) override;
  ExprRef createLNot(ExprRef e) override;
  ExprRef createIte(ExprRef expr_cond, ExprRef expr_true, ExprRef expr_false) override;

  static ExprBuilder* create();
};

class SymbolicExprBuilder : public ExprBuilder {
public:
  ExprRef createConcat(ExprRef l, ExprRef r) override;
  ExprRef createAdd(ExprRef l, ExprRef r) override;
  ExprRef createSub(ExprRef l, ExprRef r) override;
  ExprRef createMul(ExprRef l, ExprRef r) override;
  ExprRef createSDiv(ExprRef l, ExprRef r) override;
  ExprRef createUDiv(ExprRef l, ExprRef r) override;
  ExprRef createAnd(ExprRef l, ExprRef r) override;
  ExprRef createOr(ExprRef l, ExprRef r) override;
  ExprRef createXor(ExprRef l, ExprRef r) override;
  ExprRef createEqual(ExprRef l, ExprRef r) override;
  ExprRef createDistinct(ExprRef l, ExprRef r) override;
  ExprRef createLOr(ExprRef l, ExprRef r) override;
  ExprRef createLAnd(ExprRef l, ExprRef r) override;
  ExprRef createLNot(ExprRef e);
  ExprRef createIte(
    ExprRef expr_cond,
    ExprRef expr_true,
    ExprRef expr_false) override;
  ExprRef createExtract(ExprRef op, UINT32 index, UINT32 bits) override;

  static ExprBuilder* create();

private:
  ExprRef createAdd(ConstantExprRef l, NonConstantExprRef r);
  ExprRef createAdd(NonConstantExprRef l, NonConstantExprRef r);
  ExprRef createSub(ConstantExprRef l, NonConstantExprRef r);
  ExprRef createSub(NonConstantExprRef l, NonConstantExprRef r);
  ExprRef createMul(ConstantExprRef l, NonConstantExprRef r);
  ExprRef createAnd(ConstantExprRef l, NonConstantExprRef r);
  ExprRef createAnd(NonConstantExprRef l, NonConstantExprRef r);
  ExprRef createOr(ConstantExprRef l, NonConstantExprRef r);
  ExprRef createOr(NonConstantExprRef l, NonConstantExprRef r);
  ExprRef createXor(NonConstantExprRef l, NonConstantExprRef r);
  ExprRef createSDiv(NonConstantExprRef l, ConstantExprRef r);
  ExprRef createUDiv(NonConstantExprRef l, ConstantExprRef r);

  ExprRef simplifyLNot(ExprRef);
  ExprRef simplifyExclusiveExpr(ExprRef l, ExprRef r);
  };

class PruneExprBuilder : public ExprBuilder {
  public:
    // {BEGIN:FUZZ}
    ExprRef createZExt(ExprRef e, UINT32 bits) override;
    ExprRef createSExt(ExprRef e, UINT32 bits) override;
    ExprRef createAdd(ExprRef l, ExprRef r) override;
    ExprRef createSub(ExprRef l, ExprRef r) override;
    ExprRef createMul(ExprRef l, ExprRef r) override;
    ExprRef createUDiv(ExprRef l, ExprRef r) override;
    ExprRef createSDiv(ExprRef l, ExprRef r) override;
    ExprRef createURem(ExprRef l, ExprRef r) override;
    ExprRef createSRem(ExprRef l, ExprRef r) override;
    ExprRef createNeg(ExprRef e) override;
    ExprRef createNot(ExprRef e) override;
    ExprRef createAnd(ExprRef l, ExprRef r) override;
    ExprRef createOr(ExprRef l, ExprRef r) override;
    ExprRef createXor(ExprRef l, ExprRef r) override;
    ExprRef createShl(ExprRef l, ExprRef r) override;
    ExprRef createLShr(ExprRef l, ExprRef r) override;
    ExprRef createAShr(ExprRef l, ExprRef r) override;
    ExprRef createLOr(ExprRef l, ExprRef r) override;
    ExprRef createLAnd(ExprRef l, ExprRef r) override;
    ExprRef createLNot(ExprRef e) override;
    ExprRef createIte(ExprRef expr_cond, ExprRef expr_true, ExprRef expr_false) override;
    // {END:FUZZ}

    static ExprBuilder* create();
};

extern ExprBuilder *g_expr_builder;

} // namespace qsym

#endif // QSYM_EXPR_BUILDER_H_

