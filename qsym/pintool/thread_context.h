#ifndef QSYM_THREAD_CONTEXT_H_
#define QSYM_THREAD_CONTEXT_H_

#include "pin.H"

#include "call_stack_manager.h"
#include "flags.h"
#include "memory.h"
#include "third_party/libdft/syscall_desc.h"

namespace qsym {

extern Memory       g_memory;
extern SyscallDesc  kSyscallDesc[kSyscallMax];

namespace {

INT32 getMaxRegAddr() {
  INT32 idx = 0;
  for (INT32 r = REG_GR_BASE; r < REG_LAST; r++) {
    REG reg = (REG)r;
    if (isInterestingReg(reg))
      idx += REG_Size(reg);
  }
  return idx;
}

} // namespace


inline llvm::APInt getRegValue(const CONTEXT* ctx, REG reg) {
  unsigned size = REG_Size(reg);
  unsigned num_words = (size + sizeof(uint64_t) - 1) / sizeof(uint64_t);
  uint64_t value[num_words];
  PIN_GetContextRegval(ctx, reg, (UINT8*)value);
  return llvm::APInt(size * 8, num_words, value);
}

inline ExprRef getRegValueExpr(const CONTEXT* ctx, REG reg) {
  llvm::APInt v = getRegValue(ctx, reg);
  return g_expr_builder->createConstant(v, REG_Size(reg) * CHAR_BIT);
}

class ThreadContext {
  public:
    ThreadContext() {
      initializeMapRegToAddr();
    }

    void onSyscallEnter(CONTEXT* ctx, SYSCALL_STANDARD std);
    void onSyscallExit(CONTEXT* ctx, SYSCALL_STANDARD std);

    inline void clearExprFromReg(REG r, INT32 off=0, INT32 size=-1) {
      ADDRINT addr = regToRegAddr(r) + off;
      if (size == -1)
        size = REG_Size(r);
#ifdef __x86_64__
      if (REG_is_gr32(r) && off == 0) {
        REG full_reg = REG_FullRegName(r);
        // From intel manual
        // If 32-bit opreands generate 32-bit results
        // zero-extended to a 64-bit result
        // in the destination general-purpose register.
        size = REG_Size(full_reg);
      }
#endif

      switch (size) {
        case 1:
          clearExprFromRegAddr(addr);
          break;
        default:
          clearExprFromRegAddr(addr, size);
          break;
      }
    }

    inline void setExprToReg(REG r, ExprRef e) {
      setExprToReg(r, e, 0, REG_Size(r));
    }

    inline void setExprToReg(REG r, ExprRef e, INT32 off, INT32 size) {
#ifdef __x86_64__
      if (REG_is_gr32(r) && off == 0) {
        REG full_reg = REG_FullRegName(r);
        // From intel manual
        // If 32-bit opreands generate 32-bit results
        // zero-extended to a 64-bit result
        // in the destination general-purpose register.
        for (INT32 i = 4; i < 8; i++)
          clearExprFromReg(full_reg, i, 1);
      }
#endif

      ADDRINT addr = regToRegAddr(r) + off;
      switch (size) {
        case 1:
          setExprToRegAddr(addr, e);
          break;
        case 2:
        case 4:
        case 8:
        case 16:
        case 32:
          setExprToRegAddr(addr, size, e);
          break;
        default:
          LOG_FATAL("invalid size: " + std::to_string(size) + "\n");
      }
    }

    inline ExprRef getExprFromReg(const CONTEXT *ctx, REG r) {
      ExprRef e = getExprFromReg(ctx, r, 0, REG_Size(r));
#ifdef CONFIG_TRACE
      trace_getExprFromReg(e, ctx, r);
#endif
      return e;
    }

    inline ExprRef getExprFromRegAlways(
        const CONTEXT *ctx,
        REG r,
        INT32 off,
        INT32 size) {
      ADDRINT addr = regToRegAddr(r) + off;

      switch (size) {
        case 1:
          return getExprFromRegAddr(addr);
        case 2:
        case 4:
        case 8:
        case 16:
        case 32:
          return getExprFromRegAddr(ctx, addr, size);
        default:
          UNREACHABLE();
          return NULL;
      }
    }

    inline ExprRef getExprFromReg(
        const CONTEXT *ctx,
        REG r,
        INT32 off,
        INT32 size) {
      return getExprFromRegAlways(ctx, r, off, size);
    }

    ExprRef getAddrExpr(const CONTEXT* ctx,
        REG base, REG index, ADDRINT disp, UINT32 scale);

    inline void invalidateEflags(OpKind op_kind) {
      eflags_.invalidate(op_kind);
    }

    inline void setEflags(OpKind kind, ExprRef expr_result, ExprRef expr_dst, ExprRef expr_src) {
      eflags_.set(kind, expr_result, expr_dst, expr_src);
    }

    inline ExprRef computeJcc(const CONTEXT* ctx, JccKind jcc_c, bool inv) {
      return eflags_.computeJcc(ctx, jcc_c, inv);
    }

    inline ExprRef computeJccAsBV(const CONTEXT* ctx, JccKind jcc_c, bool inv, INT32 size) {
      return eflags_.computeJccAsBV(ctx, jcc_c, inv, size);
    }

    inline ExprRef computeCfAsExpr(const CONTEXT* ctx, INT32 size) {
      return eflags_.computeJccAsBV(ctx, JCC_B, false, size);
    }

  protected:
    SyscallContext syscall_ctx_;
    Eflags eflags_;
    INT32 map_reg_to_addr_[REG_LAST + 1];
    REG *map_addr_to_reg_;
    ExprRef* reg_exprs_;
    INT32 addr_limit_;
    //ExprRef reg_exprs_[(REG_GR_LAST + 1) * kRegSize];

    void clearArgs(SyscallDesc& syscall_desc);

    inline void clearExprFromRegAddr(ADDRINT addr) {
      ExprRef* ptr = getExprPtrFromRegAddr(addr);
      ExprRef e = *ptr;
      if (e != NULL) {
        *ptr = NULL;
      }
    }


    inline void clearExprFromRegAddr(ADDRINT addr, INT32 size) {
      for (int i = 0; i < size; i++)
        clearExprFromRegAddr(addr + i);
    }

    inline void setExprToRegAddr(ADDRINT addr, INT32 size, ExprRef e) {
      if (e == NULL)
        clearExprFromRegAddr(addr, size);
      else {
        for (INT32 i = 0; i < size; i++) {
          ExprRef new_expr = g_expr_builder->createExtract(e, i * 8, 8);
          setExprToRegAddr(addr + i, new_expr);
        }
      }
    }

    inline void setExprToRegAddr(ADDRINT addr, ExprRef e) {
      if (e == NULL)
        clearExprFromRegAddr(addr);
      else {
        clearExprFromRegAddr(addr);
        *getExprPtrFromRegAddr(addr) = e;
      }
    }

    inline ExprRef getExprFromRegAddr(ADDRINT addr) {
      // similar with memory case
      return *getExprPtrFromRegAddr(addr);
    }

    inline ExprRef getExprFromRegAddr(const CONTEXT* ctx, ADDRINT addr, INT32 size) {
      if (!isSymbolicReg(addr, size))
        return NULL;

      REG r = regAddrToReg(addr);
      UINT8 val[REG_Size(r)];
      std::list<ExprRef> exprs;
      PIN_GetContextRegval(ctx, r, val);

      for (INT32 i = 0; i < size; i++) {
        ExprRef e_partial = getExprFromRegAddr(addr + i);
        if (e_partial == NULL)
          e_partial = g_expr_builder->createConstant(val[i], 8);
        exprs.push_front(e_partial);
      }

      ExprRef e = g_expr_builder->createConcat(exprs);

      if (e != NULL && e->isConcrete()) {
        clearExprFromRegAddr(addr, size);
        return NULL;
      }
      return e;
    }

    inline ExprRef* getExprPtrFromRegAddr(ADDRINT addr) {
      return &reg_exprs_[addr];
    }

    bool isSymbolicReg(ADDRINT addr, INT32 size) {
      for (INT32 i = 0; i < size; i++)
        if (getExprFromRegAddr(addr + i))
          return true;
      return false;
    }

    void initializeMapRegToAddr() {
      INT32 max_reg_addr = getMaxRegAddr();
      memset(map_reg_to_addr_, -1, sizeof(map_reg_to_addr_));
      reg_exprs_ = (ExprRef*)safeCalloc(1, sizeof(ExprRef) * max_reg_addr);
      map_addr_to_reg_ = (REG*)safeCalloc(1, sizeof(REG) * max_reg_addr);

      INT32 idx = 0;
      for (INT32 r = REG_GR_BASE; r < REG_LAST; r++) {
        REG reg = (REG)r;
        if (isInterestingReg(reg)) {
          map_reg_to_addr_[reg] = idx;
          map_addr_to_reg_[idx] = reg;
          idx += REG_Size(reg);
        }
      }
    }

    inline ADDRINT regToRegAddr(REG r) {
      REG full_reg = REG_FullRegName(r);
      INT32 addr = map_reg_to_addr_[full_reg];
      if (addr == -1)
        LOG_FATAL("invalid register: " + REG_StringShort(r) + "\n");
      if (REG_is_Upper8(r))
        addr += 1;
      return addr;
    }

    inline REG regAddrToReg(ADDRINT addr) {
      REG r = map_addr_to_reg_[addr];
      if (r == REG_INVALID())
        LOG_FATAL("invalid addr: " + hexstr(addr) + "\n");
      return r;
    }

    void trace_GetExprFromReg(ExprRef e, const CONTEXT* ctx, REG r) const;
};
} // namespace qsym

#endif // QSYM_THREAD_CONTEXT_H_
