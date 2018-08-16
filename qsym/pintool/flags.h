#ifndef QSYM_FLAGS_H_
#define QSYM_FLAGS_H_

#include "pin.H"
#include "memory.h"

namespace qsym {

enum JccKind {
  JCC_B,
  JCC_BE,
  JCC_L,
  JCC_LE,
  JCC_O,
  JCC_S,
  JCC_P,
  JCC_Z,
};

enum OpKind {
  // from QEMU
  CC_OP_ADD,
  CC_OP_SUB,
  CC_OP_LOGIC,
  CC_OP_INC,
  CC_OP_DEC,
  CC_OP_SHL,
  CC_OP_SHR,
  CC_OP_ROR,
  CC_OP_ROL,
  CC_OP_SMUL,
  CC_OP_UMUL,
  CC_OP_BT,
  CC_OP_LAST
};

enum Eflag {
  // bad name
  EFLAGS_CF = (1 << 0),
  EFLAGS_PF = (1 << 2),
  EFLAGS_AF = (1 << 4),
  EFLAGS_ZF = (1 << 6),
  EFLAGS_SF = (1 << 7),
  EFLAGS_OF = (1 << 11),
  EFLAGS_LAST
};

enum InstrType {
  INSTR_ALL_FLAGS,
  INSTR_ALL_BUT_NOT_CF,
  INSTR_CF_AND_OF,
  INSTR_LAST = INSTR_CF_AND_OF
};


namespace {
inline UINT32 getAffectedEflagsFromOpKind(OpKind op_kind) {
  switch (op_kind) {
    case CC_OP_ADD:
    case CC_OP_SUB:
    case CC_OP_LOGIC:
    case CC_OP_SHL:
    case CC_OP_SHR:
    case CC_OP_SMUL:
    case CC_OP_UMUL:
      return EFLAGS_CF
        | EFLAGS_PF
        | EFLAGS_ZF
        | EFLAGS_SF
        | EFLAGS_OF;
    case CC_OP_BT:
      return EFLAGS_CF;
    case CC_OP_INC:
    case CC_OP_DEC:
      return EFLAGS_PF
        | EFLAGS_ZF
        | EFLAGS_SF
        | EFLAGS_OF;
    case CC_OP_ROR:
    case CC_OP_ROL:
      return EFLAGS_CF
        | EFLAGS_OF;
    default:
      UNREACHABLE();
      return 0;
  }
}

inline UINT32 getUndefinedEflagsFromOpKind(OpKind op_kind) {
  switch (op_kind) {
    case CC_OP_BT:
      return EFLAGS_OF
        | EFLAGS_SF
        | EFLAGS_AF
        | EFLAGS_PF;
    case CC_OP_ADD:
    case CC_OP_SUB:
    case CC_OP_LOGIC:
    case CC_OP_SHL:
    case CC_OP_SHR:
    case CC_OP_SMUL:
    case CC_OP_UMUL:
    case CC_OP_INC:
    case CC_OP_DEC:
    case CC_OP_ROR:
    case CC_OP_ROL:
      return 0;
    default:
      UNREACHABLE();
      return 0;
  }
}

inline UINT32 getEflagsFromOpKind(OpKind op_kind) {
  return getAffectedEflagsFromOpKind(op_kind)
    | getUndefinedEflagsFromOpKind(op_kind);
}

inline UINT32 getEflagsFromJcc(JccKind jcc_c) {
  switch (jcc_c) {
    case JCC_B:
      return EFLAGS_CF;
    case JCC_BE:
      return EFLAGS_CF | EFLAGS_ZF;
    case JCC_L:
      return EFLAGS_SF | EFLAGS_OF;
    case JCC_LE:
      return EFLAGS_SF | EFLAGS_OF | EFLAGS_ZF;
    case JCC_S:
      return EFLAGS_SF;
    case JCC_Z:
      return EFLAGS_ZF;
    case JCC_O:
      return EFLAGS_OF;
    case JCC_P:
      return EFLAGS_PF;
    default:
      UNREACHABLE();
      return 0;
  }
}

} // anonymous namespace

class FlagOperation {
 public:
  FlagOperation();
  virtual ~FlagOperation();

  void set(OpKind kind, ExprRef expr_result,
    ExprRef expr_dst, ExprRef expr_src);

  UINT32 flags() { return flags_; }

  inline ExprRef expr_result() { return expr_result_; }
  inline ExprRef expr_src() { return expr_src_; }
  inline ExprRef expr_dst() { return expr_dst_; }
  inline OpKind kind() { return kind_; }
  inline INT32 timestamp() { return timestamp_; }

  virtual ExprRef computeCF();
  virtual ExprRef computeOF();
  virtual ExprRef computeSF();
  virtual ExprRef computeZF();
  virtual ExprRef computePF();

 protected:
  OpKind kind_;
  UINT32 flags_;
  INT32 timestamp_;
  ExprRef expr_result_;
  ExprRef expr_dst_;
  ExprRef expr_src_;
};

class AddFlagOperation : public FlagOperation {
 public:
   ExprRef computeCF() override;
   ExprRef computeOF() override;
};

class SubFlagOperation : public FlagOperation {
 public:
   ExprRef computeCF() override;
   ExprRef computeOF() override;
};

class LogicFlagOperation : public FlagOperation {
 public:
   ExprRef computeCF() override;
   ExprRef computeOF() override;
};

class IncFlagOperation : public FlagOperation {
 public:
   ExprRef computeOF() override;
};

class DecFlagOperation : public FlagOperation {
 public:
   ExprRef computeOF() override;
};

class ShiftFlagOperation : public FlagOperation {
 public:
   ExprRef computeOF() override;
};

class ShlFlagOperation : public ShiftFlagOperation {
 public:
   ExprRef computeCF() override;
};

class ShrFlagOperation : public ShiftFlagOperation {
 public:
   ExprRef computeCF() override;
};

class RolFlagOperation : public FlagOperation {
 public:
   ExprRef computeCF() override;
   ExprRef computeOF() override;
};

class RorFlagOperation : public FlagOperation {
 public:
   ExprRef computeCF() override;
   ExprRef computeOF() override;
};

class MulFlagOperation : public FlagOperation {
 protected:
   ExprRef _computeOF(bool sign);
};

class SMulFlagOperation : public MulFlagOperation {
 public:
   ExprRef computeOF() override;
   ExprRef computeCF() override;
};

class UMulFlagOperation : public MulFlagOperation {
 public:
   ExprRef computeOF() override;
   ExprRef computeCF() override;
};

class BtFlagOperation : public FlagOperation {
 public:
   ExprRef computeCF() override;
};

class Eflags {
public:
  Eflags();
  ~Eflags();

  inline void invalidate(OpKind op_kind) {
    valid_set_ &= ~getEflagsFromOpKind(op_kind);
  }

  inline void validate(OpKind op_kind) {
    valid_set_ &= ~getUndefinedEflagsFromOpKind(op_kind);
    valid_set_ |= getAffectedEflagsFromOpKind(op_kind);
  }

  bool isValid(JccKind jcc) {
    // if jcc uses two flags from different operations,
    // then it could be incorrect, but it's not normal
    UINT32 eflags = getEflagsFromJcc(jcc);
    return (valid_set_ & eflags) == eflags;
  }

  void set(OpKind kind, ExprRef expr_result,
      ExprRef expr_dst, ExprRef expr_src);

  ExprRef computeJcc(const CONTEXT* ctx, JccKind jcc_c, bool inv);
  ExprRef computeJccAsBV(const CONTEXT* ctx, JccKind jcc_c, bool inv, INT32 size);

 private:
  UINT32 valid_set_;
  FlagOperation *operations_[CC_OP_LAST];
  OpKind op_kinds_[CC_OP_LAST];
  UINT32 start_;

  ExprRef computeFastJcc(const CONTEXT* ctx, JccKind jcc_c, bool inv);
  ExprRef computeSlowJcc(const CONTEXT* ctx, JccKind jcc_c, bool inv);
  ExprRef computeFlag(Eflag flag);

};

} // namespace qsym

#endif
