#ifndef QSYM_ANALYSIS_INSTRUCTION_H_
#define QSYM_ANALYSIS_INSTRUCTION_H_

#include "pin.H"
#include "compiler.h"
#include "expr.h"
#include "flags.h"

namespace qsym {
enum {
  OP_0 = 0,
  OP_1 = 1,
  OP_2 = 2,
  OP_3 = 3,
  OP_4 = 4,
  OP_5 = 5,
};

void analyzeBBL(BBL bbl);

void analyzeDefault(INS ins);
void analyzeBinary(INS ins, Kind kind, bool write);
void analyzeCarry(INS ins, Kind kind);

void analyzeBs(INS ins, bool inv);
void analyzeBswap(INS ins);
void analyzeBt(INS ins, Kind kind);
void analyzeCall(INS ins);
void analyzeCbw(INS ins);
void analyzeCwd(INS ins);
void analyzeCmov(INS ins, JccKind jcc_kind, bool inv);
void analyzeCmps(INS ins, UINT width);
void analyzeCmpxchg(INS ins);
void analyzeCpuid(INS ins);
void analyzeDec(INS ins);
void analyzeDiv(INS ins, bool sign);
void analyzeFild(INS ins);
void analyzeClear(INS ins);
void analyzeIMul(INS ins);
void analyzeIncDec(INS ins, Kind kind);
void analyzeLea(INS ins);
void analyzeLeave(INS ins);
void analyzeMov(INS ins);
void analyzeMovh(INS ins);
void analyzeMovlz(INS ins, UINT width);
void analyzeMovs(INS ins, UINT width);
void analyzeMovsx(INS ins);
void analyzeMovzx(INS ins);
void analyzeMul(INS ins);
void analyzeNegNot(INS ins, Kind kind);
void analyzeJcc(INS ins, JccKind jcc_kind, bool inv);
void analyzeJmp(INS ins);
void analyzePack(INS ins, UINT width, bool sign);
void analyzePalignr(INS ins);
void analyzeParith(INS ins, Kind kind, UINT width);
void analyzePcmp(INS ins, Kind kind, UINT width);
void analyzePcmpxchg(INS ins, Kind kind, UINT width);
void analyzePextr(INS ins, UINT width);
void analyzePmaddwd(INS ins);
void analyzePmul(INS ins, UINT width);
void analyzePmulhw(INS ins, bool sign);
void analyzePmovmskb(INS ins);
void analyzePop(INS ins);
void analyzePshufb(INS ins);
void analyzePshuf(INS ins, UINT width);
void analyzePshufw(INS ins, bool high);
void analyzePshift(INS ins, Kind kind, UINT width);
void analyzePunpckh(INS ins, UINT width);
void analyzePunpckl(INS ins, UINT width);
void analyzePush(INS ins);
void analyzeRdtsc(INS ins);
void analyzeRdtscp(INS ins);
void analyzeScas(INS ins, UINT width);
void analyzeSet(INS ins, JccKind jcc_kind, bool inv);
void analyzeShift(INS ins, Kind kind);
void analyzeShiftd(INS ins, Kind kind);
void analyzeStos(INS ins);
void analyzeXadd(INS ins);
void analyzeXchg(INS ins);
void analyzeTernary(INS ins, Kind kind);

void analyzeVmovl(INS ins);
void analyzeVparith(INS ins, Kind kind, UINT width);
void analyzeVinserti128(INS ins);
void analyzeVpmaddwd(INS ins);
void analyzeVpcmp(INS ins, Kind kind, UINT width);
void analyzeVpcmpxchg(INS ins, Kind kind, UINT width);
void analyzeVpmulhw(INS ins, bool sign);
void analyzeVpmul(INS ins, UINT width);
void analyzeVpshufb(INS ins);
void analyzeVpshift(INS ins, Kind kind, UINT width);
void analyzeVpack(INS ins, UINT width, bool sign);

void analyzeRet(INS ins);

} // namespace qsym

#endif // QSYM_ANALYZE_INSTRUCTION_H_
