#include <iostream>
#include "analysis.h"
#include "analysis_instruction.h"
#include "flags.h"

namespace qsym {

void
analyzeTrace(TRACE trace, VOID *v)
{
  for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
    analyzeBBL(bbl);
    for (INS ins = BBL_InsHead(bbl);
        INS_Valid(ins);
        ins = INS_Next(ins)) {
      analyzeInstruction(ins);
    }
  }
}

void
analyzeInstruction(INS ins) {
  // use XED to decode the instruction and extract its opcode
	xed_iclass_enum_t ins_indx = (xed_iclass_enum_t)INS_Opcode(ins);

	if (ins_indx <= XED_ICLASS_INVALID ||
				ins_indx >= XED_ICLASS_LAST) {
    LOG_WARN("unknown opcode (opcode=" + decstr(ins_indx) + ")" + "\n");
		return;
	}

  switch (ins_indx) {
  // XED_ICLASS_AAA,
  // XED_ICLASS_AAD,
  // XED_ICLASS_AAM,
  // XED_ICLASS_AAS,
  case XED_ICLASS_ADC:
    analyzeCarry(ins, Add);
    break;
  // XED_ICLASS_ADCX,
  case XED_ICLASS_ADD:
    analyzeBinary(ins, Add, true);
    break;
  // XED_ICLASS_ADDPD,
  // XED_ICLASS_ADDPS,
  // XED_ICLASS_ADDSD,
  // XED_ICLASS_ADDSS,
  // XED_ICLASS_ADDSUBPD,
  // XED_ICLASS_ADDSUBPS,
  // XED_ICLASS_ADOX,
  // XED_ICLASS_AESDEC,
  // XED_ICLASS_AESDECLAST,
  // XED_ICLASS_AESENC,
  // XED_ICLASS_AESENCLAST,
  // XED_ICLASS_AESIMC,
  // XED_ICLASS_AESKEYGENASSIST,
  case XED_ICLASS_AND:
    analyzeBinary(ins, And, true);
    break;
  // XED_ICLASS_ANDN,
  // XED_ICLASS_ANDNPD,
  // XED_ICLASS_ANDNPS,
  case XED_ICLASS_ANDPD:
  case XED_ICLASS_ANDPS:
    analyzeBinary(ins, And, true);
    break;
  // XED_ICLASS_ARPL,
  // XED_ICLASS_BEXTR,
  // XED_ICLASS_BEXTR_XOP,
  // XED_ICLASS_BLCFILL,
  // XED_ICLASS_BLCI,
  // XED_ICLASS_BLCIC,
  // XED_ICLASS_BLCMSK,
  // XED_ICLASS_BLCS,
  // XED_ICLASS_BLENDPD,
  // XED_ICLASS_BLENDPS,
  // XED_ICLASS_BLENDVPD,
  // XED_ICLASS_BLENDVPS,
  // XED_ICLASS_BLSFILL,
  // XED_ICLASS_BLSI,
  // XED_ICLASS_BLSIC,
  // XED_ICLASS_BLSMSK,
  // XED_ICLASS_BLSR,
  // XED_ICLASS_BNDCL,
  // XED_ICLASS_BNDCN,
  // XED_ICLASS_BNDCU,
  // XED_ICLASS_BNDLDX,
  // XED_ICLASS_BNDMK,
  // XED_ICLASS_BNDMOV,
  // XED_ICLASS_BNDSTX,
  // XED_ICLASS_BOUND,
  case XED_ICLASS_BSF:
    analyzeBs(ins, false);
    break;
  case XED_ICLASS_BSR:
    analyzeBs(ins, true);
    break;
  case XED_ICLASS_BSWAP:
    analyzeBswap(ins);
    break;
  case XED_ICLASS_BT:
    analyzeBt(ins, Invalid);
    break;
  case XED_ICLASS_BTC:
    analyzeBt(ins, Xor);
    break;
  case XED_ICLASS_BTR:
    analyzeBt(ins, And);
    break;
  case XED_ICLASS_BTS:
    analyzeBt(ins, Or);
    break;
  // XED_ICLASS_BZHI,
  case XED_ICLASS_CALL_FAR:
  case XED_ICLASS_CALL_NEAR:
    analyzeCall(ins);
    break;
  case XED_ICLASS_CBW:
    analyzeCbw(ins);
    break;
  case XED_ICLASS_CDQ:
    analyzeCwd(ins);
    break;
  case XED_ICLASS_CDQE:
    analyzeCbw(ins);
    break;
  // XED_ICLASS_CLAC,
  // XED_ICLASS_CLC,
  // XED_ICLASS_CLD,
  // XED_ICLASS_CLFLUSH,
  // XED_ICLASS_CLFLUSHOPT,
  // XED_ICLASS_CLGI,
  // XED_ICLASS_CLI,
  // XED_ICLASS_CLTS,
  // XED_ICLASS_CMC,
  case XED_ICLASS_CMOVB:
    analyzeCmov(ins, JCC_B, false);
    break;
  case XED_ICLASS_CMOVBE:
    analyzeCmov(ins, JCC_BE, false);
    break;
  case XED_ICLASS_CMOVL:
    analyzeCmov(ins, JCC_L, false);
    break;
  case XED_ICLASS_CMOVLE:
    analyzeCmov(ins, JCC_LE, false);
    break;
  case XED_ICLASS_CMOVNB:
    analyzeCmov(ins, JCC_B, true);
    break;
  case XED_ICLASS_CMOVNBE:
    analyzeCmov(ins, JCC_BE, true);
    break;
  case XED_ICLASS_CMOVNL:
    analyzeCmov(ins, JCC_L, true);
    break;
  case XED_ICLASS_CMOVNLE:
    analyzeCmov(ins, JCC_LE, true);
    break;
  case XED_ICLASS_CMOVNO:
    analyzeCmov(ins, JCC_O, true);
    break;
  case XED_ICLASS_CMOVNP:
    analyzeCmov(ins, JCC_P, true);
    break;
  case XED_ICLASS_CMOVNS:
    analyzeCmov(ins, JCC_S, true);
    break;
  case XED_ICLASS_CMOVNZ:
    analyzeCmov(ins, JCC_Z, true);
    break;
  case XED_ICLASS_CMOVO:
    analyzeCmov(ins, JCC_O, false);
    break;
  case XED_ICLASS_CMOVP:
    analyzeCmov(ins, JCC_P, false);
    break;
  case XED_ICLASS_CMOVS:
    analyzeCmov(ins, JCC_S, false);
    break;
  case XED_ICLASS_CMOVZ:
    analyzeCmov(ins, JCC_Z, false);
    break;
  case XED_ICLASS_CMP:
    analyzeBinary(ins, Sub, false);
    break;
  // XED_ICLASS_CMPPD,
  // XED_ICLASS_CMPPS,
  case XED_ICLASS_CMPSB:
    analyzeCmps(ins, 1);
    break;
  case XED_ICLASS_CMPSD:
    analyzeCmps(ins, 4);
    break;
  // XED_ICLASS_CMPSD_XMM,
  case XED_ICLASS_CMPSQ:
    analyzeCmps(ins, 8);
    break;
  // XED_ICLASS_CMPSS
  case XED_ICLASS_CMPSW:
    analyzeCmps(ins, 2);
    break;
  case XED_ICLASS_CMPXCHG:
    analyzeCmpxchg(ins);
    break;
  // XED_ICLASS_CMPXCHG16B,
  // XED_ICLASS_CMPXCHG8B,
  // XED_ICLASS_COMISD,
  // XED_ICLASS_COMISS,
  case XED_ICLASS_CPUID:
    analyzeCpuid(ins);
    break;
  case XED_ICLASS_CQO:
    analyzeCwd(ins);
    break;
  // XED_ICLASS_CRC32,
  // XED_ICLASS_CVTDQ2PD,
  // XED_ICLASS_CVTDQ2PS,
  // XED_ICLASS_CVTPD2DQ,
  // XED_ICLASS_CVTPD2PI,
  // XED_ICLASS_CVTPD2PS,
  // XED_ICLASS_CVTPI2PD,
  // XED_ICLASS_CVTPI2PS,
  // XED_ICLASS_CVTPS2DQ,
  // XED_ICLASS_CVTPS2PD,
  // XED_ICLASS_CVTPS2PI,
  // XED_ICLASS_CVTSD2SI,
  // XED_ICLASS_CVTSD2SS,
  // XED_ICLASS_CVTSI2SD,
  // XED_ICLASS_CVTSI2SS,
  // XED_ICLASS_CVTSS2SD,
  // XED_ICLASS_CVTSS2SI,
  // XED_ICLASS_CVTTPD2DQ,
  // XED_ICLASS_CVTTPD2PI,
  // XED_ICLASS_CVTTPS2DQ,
  // XED_ICLASS_CVTTPS2PI,
  // XED_ICLASS_CVTTSD2SI,
  // XED_ICLASS_CVTTSS2SI,
  case XED_ICLASS_CWD:
    analyzeCwd(ins);
    break;
  case XED_ICLASS_CWDE:
    analyzeCbw(ins);
    break;
  // XED_ICLASS_DAA,
  // XED_ICLASS_DAS,
  case XED_ICLASS_DEC:
    analyzeIncDec(ins, Sub);
    break;
  case XED_ICLASS_DIV:
    analyzeDiv(ins, false);
    break;
  // XED_ICLASS_DIVPD,
  // XED_ICLASS_DIVPS,
  // XED_ICLASS_DIVSD,
  // XED_ICLASS_DIVSS,
  // XED_ICLASS_DPPD,
  // XED_ICLASS_DPPS,
  // XED_ICLASS_EMMS,
  // XED_ICLASS_ENCLS,
  // XED_ICLASS_ENCLU,
  // XED_ICLASS_ENTER,
  // XED_ICLASS_EXTRACTPS,
  // XED_ICLASS_EXTRQ,
  // XED_ICLASS_F2XM1,
  // XED_ICLASS_FABS,
  // XED_ICLASS_FADD,
  // XED_ICLASS_FADDP,
  // XED_ICLASS_FBLD,
  // XED_ICLASS_FBSTP,
  // XED_ICLASS_FCHS,
  // XED_ICLASS_FCMOVB,
  // XED_ICLASS_FCMOVBE,
  // XED_ICLASS_FCMOVE,
  // XED_ICLASS_FCMOVNB,
  // XED_ICLASS_FCMOVNBE,
  // XED_ICLASS_FCMOVNE,
  // XED_ICLASS_FCMOVNU,
  // XED_ICLASS_FCMOVU,
  // XED_ICLASS_FCOM,
  // XED_ICLASS_FCOMI,
  // XED_ICLASS_FCOMIP,
  // XED_ICLASS_FCOMP,
  // XED_ICLASS_FCOMPP,
  // XED_ICLASS_FCOS,
  // XED_ICLASS_FDECSTP,
  // XED_ICLASS_FDISI8087_NOP,
  // XED_ICLASS_FDIV,
  // XED_ICLASS_FDIVP,
  // XED_ICLASS_FDIVR,
  // XED_ICLASS_FDIVRP,
  // XED_ICLASS_FEMMS,
  // XED_ICLASS_FENI8087_NOP,
  // XED_ICLASS_FFREE,
  // XED_ICLASS_FFREEP,
  // XED_ICLASS_FIADD,
  // XED_ICLASS_FICOM,
  // XED_ICLASS_FICOMP,
  // XED_ICLASS_FIDIV,
  // XED_ICLASS_FIDIVR,
  // XED_ICLASS_FILD,
  // XED_ICLASS_FIMUL,
  // XED_ICLASS_FINCSTP,
  case XED_ICLASS_FIST:
    analyzeClear(ins);
    break;
  case XED_ICLASS_FISTP:
    analyzeClear(ins);
    break;
  // XED_ICLASS_FISTTP,
  // XED_ICLASS_FISUB,
  // XED_ICLASS_FISUBR,
  // XED_ICLASS_FLD,
  // XED_ICLASS_FLD1,
  // XED_ICLASS_FLDCW,
  // XED_ICLASS_FLDENV,
  // XED_ICLASS_FLDL2E,
  // XED_ICLASS_FLDL2T,
  // XED_ICLASS_FLDLG2,
  // XED_ICLASS_FLDLN2,
  // XED_ICLASS_FLDPI,
  // XED_ICLASS_FLDZ,
  // XED_ICLASS_FMUL,
  // XED_ICLASS_FMULP,
  // XED_ICLASS_FNCLEX,
  // XED_ICLASS_FNINIT,
  // XED_ICLASS_FNOP,
  // XED_ICLASS_FNSAVE,
  case XED_ICLASS_FNSTCW:
    analyzeClear(ins);
    break;
  // XED_ICLASS_FNSTENV,
  case XED_ICLASS_FNSTSW:
    analyzeClear(ins);
    break;
  // XED_ICLASS_FPATAN,
  // XED_ICLASS_FPREM,
  // XED_ICLASS_FPREM1,
  // XED_ICLASS_FPTAN,
  // XED_ICLASS_FRNDINT,
  // XED_ICLASS_FRSTOR,
  // XED_ICLASS_FSCALE,
  // XED_ICLASS_FSETPM287_NOP,
  // XED_ICLASS_FSIN,
  // XED_ICLASS_FSINCOS,
  // XED_ICLASS_FSQRT,
  case XED_ICLASS_FST:
    analyzeClear(ins);
    break;
  case XED_ICLASS_FSTP:
    analyzeClear(ins);
    break;
  // XED_ICLASS_FSTPNCE,
  // XED_ICLASS_FSUB,
  // XED_ICLASS_FSUBP,
  // XED_ICLASS_FSUBR,
  // XED_ICLASS_FSUBRP,
  // XED_ICLASS_FTST,
  // XED_ICLASS_FUCOM,
  // XED_ICLASS_FUCOMI,
  // XED_ICLASS_FUCOMIP,
  // XED_ICLASS_FUCOMP,
  // XED_ICLASS_FUCOMPP,
  // XED_ICLASS_FWAIT,
  // XED_ICLASS_FXAM,
  // XED_ICLASS_FXCH,
  // XED_ICLASS_FXRSTOR,
  // XED_ICLASS_FXRSTOR64,
  // XED_ICLASS_FXSAVE,
  // XED_ICLASS_FXSAVE64,
  // XED_ICLASS_FXTRACT,
  // XED_ICLASS_FYL2X,
  // XED_ICLASS_FYL2XP1,
  // XED_ICLASS_GETSEC,
  // XED_ICLASS_HADDPD,
  // XED_ICLASS_HADDPS,
  // XED_ICLASS_HLT,
  // XED_ICLASS_HSUBPD,
  // XED_ICLASS_HSUBPS,
  case XED_ICLASS_IDIV:
    analyzeDiv(ins, true);
    break;
  case XED_ICLASS_IMUL:
    analyzeIMul(ins);
    break;
  // XED_ICLASS_IN,
  case XED_ICLASS_INC:
    analyzeIncDec(ins, Add);
    break;
  // XED_ICLASS_INSB,
  // XED_ICLASS_INSD,
  // XED_ICLASS_INSERTPS,
  // XED_ICLASS_INSERTQ,
  // XED_ICLASS_INSW,
  case XED_ICLASS_INT:
    // nothing, but clear EAX in SyscallEnter
    break;
  // XED_ICLASS_INT1,
  // XED_ICLASS_INT3,
  // XED_ICLASS_INTO,
  // XED_ICLASS_INVD,
  // XED_ICLASS_INVEPT,
  // XED_ICLASS_INVLPG,
  // XED_ICLASS_INVLPGA,
  // XED_ICLASS_INVPCID,
  // XED_ICLASS_INVVPID,
  // XED_ICLASS_IRET,
  // XED_ICLASS_IRETD,
  // XED_ICLASS_IRETQ,
  case XED_ICLASS_JB:
    analyzeJcc(ins, JCC_B, false);
    break;
  case XED_ICLASS_JBE:
    analyzeJcc(ins, JCC_BE, false);
    break;
  case XED_ICLASS_JL:
    analyzeJcc(ins, JCC_L, false);
    break;
  case XED_ICLASS_JLE:
    analyzeJcc(ins, JCC_LE, false);
    break;
  case XED_ICLASS_JMP:
  case XED_ICLASS_JMP_FAR:
    analyzeJmp(ins);
    break;
  case XED_ICLASS_JNB:
    analyzeJcc(ins, JCC_B, true);
    break;
  case XED_ICLASS_JNBE:
    analyzeJcc(ins, JCC_BE, true);
    break;
  case XED_ICLASS_JNL:
    analyzeJcc(ins, JCC_L, true);
    break;
  case XED_ICLASS_JNLE:
    analyzeJcc(ins, JCC_LE, true);
    break;
  case XED_ICLASS_JNO:
    analyzeJcc(ins, JCC_O, true);
    break;
  case XED_ICLASS_JNP:
    analyzeJcc(ins, JCC_P, true);
    break;
  case XED_ICLASS_JNS:
    analyzeJcc(ins, JCC_S, true);
    break;
  case XED_ICLASS_JNZ:
    analyzeJcc(ins, JCC_Z, true);
    break;
  case XED_ICLASS_JO:
    analyzeJcc(ins, JCC_O, false);
    break;
  case XED_ICLASS_JP:
    analyzeJcc(ins, JCC_P, false);
    break;
  // XED_ICLASS_JRCXZ,
  case XED_ICLASS_JS:
    analyzeJcc(ins, JCC_S, false);
    break;
  case XED_ICLASS_JZ:
    analyzeJcc(ins, JCC_Z, false);
    break;
  // XED_ICLASS_LAHF,
  // XED_ICLASS_LAR,
  case XED_ICLASS_LDDQU:
    analyzeMov(ins);
    break;
  // XED_ICLASS_LDMXCSR,
  // XED_ICLASS_LDS,
  case XED_ICLASS_LEA:
    analyzeLea(ins);
    break;
  case XED_ICLASS_LEAVE:
    analyzeLeave(ins);
    break;
  // XED_ICLASS_LES,
  // XED_ICLASS_LFENCE,
  // XED_ICLASS_LFS,
  // XED_ICLASS_LGDT,
  // XED_ICLASS_LGS,
  // XED_ICLASS_LIDT,
  // XED_ICLASS_LLDT,
  // XED_ICLASS_LLWPCB,
  // XED_ICLASS_LMSW,
  // XED_ICLASS_LODSB,
  // XED_ICLASS_LODSD,
  // XED_ICLASS_LODSQ,
  // XED_ICLASS_LODSW,
  // XED_ICLASS_LOOP,
  // XED_ICLASS_LOOPE,
  // XED_ICLASS_LOOPNE,
  // XED_ICLASS_LSL,
  // XED_ICLASS_LSS,
  // XED_ICLASS_LTR,
  // XED_ICLASS_LWPINS,
  // XED_ICLASS_LWPVAL,
  case XED_ICLASS_LZCNT:
    analyzeBs(ins, true);
    break;
  // XED_ICLASS_MASKMOVDQU,
  // XED_ICLASS_MASKMOVQ,
  // XED_ICLASS_MAXPD
  // XED_ICLASS_MAXPS
  // XED_ICLASS_MAXSD
  // XED_ICLASS_MAXSS
  // XED_ICLASS_MFENCE,
  // XED_ICLASS_MINPD,
  // XED_ICLASS_MINPS,
  // XED_ICLASS_MINSD,
  // XED_ICLASS_MINSS,
  // XED_ICLASS_MONITOR,
  case XED_ICLASS_MOV:
  case XED_ICLASS_MOVAPD:
  case XED_ICLASS_MOVAPS:
    analyzeMov(ins);
    break;
  // XED_ICLASS_MOVBE,
  case XED_ICLASS_MOVD:
    analyzeMovlz(ins, 4);
    break;
  // XED_ICLASS_MOVDDUP,
  case XED_ICLASS_MOVDQ2Q:
  case XED_ICLASS_MOVDQA:
  case XED_ICLASS_MOVDQU:
    analyzeMov(ins);
    break;
  // XED_ICLASS_MOVHLPS,
  case XED_ICLASS_MOVHPS:
  case XED_ICLASS_MOVHPD:
    analyzeMovh(ins);
    break;
  // XED_ICLASS_MOVLHPS:
  case XED_ICLASS_MOVLPD:
  case XED_ICLASS_MOVLPS:
    analyzeMov(ins);
    break;
  // XED_ICLASS_MOVMSKPD:
  // XED_ICLASS_MOVMSKPS:
  case XED_ICLASS_MOVNTDQ:
  case XED_ICLASS_MOVNTDQA:
  case XED_ICLASS_MOVNTI:
  case XED_ICLASS_MOVNTPD:
  case XED_ICLASS_MOVNTPS:
  case XED_ICLASS_MOVNTQ:
  case XED_ICLASS_MOVNTSD:
  case XED_ICLASS_MOVNTSS:
    analyzeMov(ins);
    break;
  case XED_ICLASS_MOVQ:
    analyzeMovlz(ins, 8);
    break;
  // XED_ICLASS_MOVQ2DQ,
  case XED_ICLASS_MOVSB:
    analyzeMovs(ins, 1);
    break;
  case XED_ICLASS_MOVSD:
    analyzeMovs(ins, 4);
    break;
  case XED_ICLASS_MOVSD_XMM:
    analyzeMov(ins);
    break;
  // XED_ICLASS_MOVSHDUP,
  // XED_ICLASS_MOVSLDUP,
  case XED_ICLASS_MOVSQ:
    analyzeMovs(ins, 8);
    break;
  // XED_ICLASS_MOVSS,
  case XED_ICLASS_MOVSW:
    analyzeMovs(ins, 2);
    break;
  case XED_ICLASS_MOVSX:
  case XED_ICLASS_MOVSXD:
    analyzeMovsx(ins);
    break;
  case XED_ICLASS_MOVUPD:
  case XED_ICLASS_MOVUPS:
    analyzeMov(ins);
    break;
  case XED_ICLASS_MOVZX:
    analyzeMovzx(ins);
    break;
  // XED_ICLASS_MOV_CR,
  // XED_ICLASS_MOV_DR,
  // XED_ICLASS_MPSADBW,
  case XED_ICLASS_MUL:
    analyzeMul(ins);
    break;
  // XED_ICLASS_MULPD,
  // XED_ICLASS_MULPS,
  // XED_ICLASS_MULSD,
  // XED_ICLASS_MULSS,
  // XED_ICLASS_MULX,
  // XED_ICLASS_MWAIT,
  case XED_ICLASS_NEG:
    analyzeNegNot(ins, Neg);
    break;
  case XED_ICLASS_NOP:
  case XED_ICLASS_NOP2:
  case XED_ICLASS_NOP3:
  case XED_ICLASS_NOP4:
  case XED_ICLASS_NOP5:
  case XED_ICLASS_NOP6:
  case XED_ICLASS_NOP7:
  case XED_ICLASS_NOP8:
  case XED_ICLASS_NOP9:
    break;
  case XED_ICLASS_NOT:
    analyzeNegNot(ins, Not);
    break;
  case XED_ICLASS_OR:
  case XED_ICLASS_ORPD:
  case XED_ICLASS_ORPS:
    analyzeBinary(ins, Or, true);
    break;
  // XED_ICLASS_OUT,
  // XED_ICLASS_OUTSB,
  // XED_ICLASS_OUTSD,
  // XED_ICLASS_OUTSW,
  // XED_ICLASS_PABSB,
  // XED_ICLASS_PABSD,
  // XED_ICLASS_PABSW,
  case XED_ICLASS_PACKSSDW:
    analyzePack(ins, 2, true);
    break;
  case XED_ICLASS_PACKSSWB:
    analyzePack(ins, 1, true);
    break;
  case XED_ICLASS_PACKUSDW:
    analyzePack(ins, 2, false);
    break;
  case XED_ICLASS_PACKUSWB:
    analyzePack(ins, 1, false);
    break;
  case XED_ICLASS_PADDB:
    analyzeParith(ins, Add, 1);
    break;
  case XED_ICLASS_PADDD:
    analyzeParith(ins, Add, 4);
    break;
  case XED_ICLASS_PADDQ:
    analyzeParith(ins, Add, 8);
    break;
  // XED_ICLASS_PADDSB,
  // XED_ICLASS_PADDSW,
  // XED_ICLASS_PADDUSB,
  // XED_ICLASS_PADDUSW,
  case XED_ICLASS_PADDW:
    analyzeParith(ins, Add, 2);
    break;
  case XED_ICLASS_PALIGNR:
    analyzePalignr(ins);
    break;
  case XED_ICLASS_PAND:
    analyzeBinary(ins, And, true);
    break;
  // XED_ICLASS_PANDN,
  // XED_ICLASS_PAUSE,
  // XED_ICLASS_PAVGB,
  // XED_ICLASS_PAVGUSB,
  // XED_ICLASS_PAVGW,
  // XED_ICLASS_PBLENDVB,
  // XED_ICLASS_PBLENDW,
  // XED_ICLASS_PCLMULQDQ,
  case XED_ICLASS_PCMPEQB:
    analyzePcmp(ins, Equal, 1);
    break;
  case XED_ICLASS_PCMPEQD:
    analyzePcmp(ins, Equal, 4);
    break;
  case XED_ICLASS_PCMPEQQ:
    analyzePcmp(ins, Equal, 8);
    break;
  case XED_ICLASS_PCMPEQW:
    analyzePcmp(ins, Equal, 2);
    break;
  // XED_ICLASS_PCMPESTRI,
  // XED_ICLASS_PCMPESTRM,
  case XED_ICLASS_PCMPGTB:
    analyzePcmp(ins, Sgt, 1);
    break;
  case XED_ICLASS_PCMPGTD:
    analyzePcmp(ins, Sgt, 4);
    break;
  case XED_ICLASS_PCMPGTQ:
    analyzePcmp(ins, Sgt, 8);
    break;
  case XED_ICLASS_PCMPGTW:
    analyzePcmp(ins, Sgt, 2);
    break;
  // XED_ICLASS_PCMPISTRI
  // XED_ICLASS_PCMPISTRM
  // XED_ICLASS_PDEP,
  // XED_ICLASS_PEXT,
  case XED_ICLASS_PEXTRB:
    analyzePextr(ins, 1);
    break;
  case XED_ICLASS_PEXTRD:
    analyzePextr(ins, 4);
    break;
  case XED_ICLASS_PEXTRQ:
    analyzePextr(ins, 8);
    break;
  case XED_ICLASS_PEXTRW:
    analyzePextr(ins, 2);
    break;
  // XED_ICLASS_PEXTRW_SSE4,
  // XED_ICLASS_PF2ID,
  // XED_ICLASS_PF2IW,
  // XED_ICLASS_PFACC,
  // XED_ICLASS_PFADD,
  // XED_ICLASS_PFCMPEQ,
  // XED_ICLASS_PFCMPGE,
  // XED_ICLASS_PFCMPGT,
  // XED_ICLASS_PFCPIT1,
  // XED_ICLASS_PFMAX,
  // XED_ICLASS_PFMIN,
  // XED_ICLASS_PFMUL,
  // XED_ICLASS_PFNACC,
  // XED_ICLASS_PFPNACC,
  // XED_ICLASS_PFRCP,
  // XED_ICLASS_PFRCPIT2,
  // XED_ICLASS_PFRSQIT1,
  // XED_ICLASS_PFSQRT,
  // XED_ICLASS_PFSUB,
  // XED_ICLASS_PFSUBR,
  // XED_ICLASS_PHADDD,
  // XED_ICLASS_PHADDSW,
  // XED_ICLASS_PHADDW,
  // XED_ICLASS_PHMINPOSUW,
  // XED_ICLASS_PHSUBD,
  // XED_ICLASS_PHSUBSW,
  // XED_ICLASS_PHSUBW,
  // XED_ICLASS_PI2FD,
  // XED_ICLASS_PI2FW,
  // XED_ICLASS_PINSRB,
  // XED_ICLASS_PINSRD,
  // XED_ICLASS_PINSRQ,
  // XED_ICLASS_PINSRW,
  // XED_ICLASS_PMADDUBSW,
  case XED_ICLASS_PMADDWD:
    analyzePmaddwd(ins);
    break;
  case XED_ICLASS_PMAXSB:
    analyzePcmpxchg(ins, Sgt, 1);
    break;
  case XED_ICLASS_PMAXSD:
    analyzePcmpxchg(ins, Sgt, 4);
    break;
  case XED_ICLASS_PMAXSW:
    analyzePcmpxchg(ins, Ugt, 2);
    break;
  case XED_ICLASS_PMAXUB:
    analyzePcmpxchg(ins, Ugt, 1);
    break;
  case XED_ICLASS_PMAXUD:
    analyzePcmpxchg(ins, Ugt, 4);
    break;
  case XED_ICLASS_PMAXUW:
    analyzePcmpxchg(ins, Ugt, 2);
    break;
  case XED_ICLASS_PMINSB:
    analyzePcmpxchg(ins, Slt, 1);
    break;
  case XED_ICLASS_PMINSD:
    analyzePcmpxchg(ins, Slt, 4);
    break;
  case XED_ICLASS_PMINSW:
    analyzePcmpxchg(ins, Slt, 2);
    break;
  case XED_ICLASS_PMINUB:
    analyzePcmpxchg(ins, Ult, 1);
    break;
  case XED_ICLASS_PMINUD:
    analyzePcmpxchg(ins, Ult, 4);
    break;
  case XED_ICLASS_PMINUW:
    analyzePcmpxchg(ins, Ult, 2);
    break;
  case XED_ICLASS_PMOVMSKB:
    analyzePmovmskb(ins);
    break;
  // XED_ICLASS_PMOVSXBD,
  // XED_ICLASS_PMOVSXBQ,
  // XED_ICLASS_PMOVSXBW,
  // XED_ICLASS_PMOVSXDQ,
  // XED_ICLASS_PMOVSXWD,
  // XED_ICLASS_PMOVSXWQ,
  // XED_ICLASS_PMOVZXBD,
  // XED_ICLASS_PMOVZXBQ,
  // XED_ICLASS_PMOVZXBW,
  // XED_ICLASS_PMOVZXDQ,
  // XED_ICLASS_PMOVZXWD,
  // XED_ICLASS_PMOVZXWQ,
  // XED_ICLASS_PMULDQ,
  // XED_ICLASS_PMULHRSW,
  // XED_ICLASS_PMULHRW,
  case XED_ICLASS_PMULHUW:
    analyzePmulhw(ins, false);
    break;
  case XED_ICLASS_PMULHW:
    analyzePmulhw(ins, true);
    break;
  case XED_ICLASS_PMULLD:
    analyzePmul(ins, 4);
    break;
  case XED_ICLASS_PMULLW:
    analyzePmul(ins, 2);
    break;
  // XED_ICLASS_PMULUDQ,
  case XED_ICLASS_POP:
    analyzePop(ins);
    break;
  // XED_ICLASS_POPA,
  // XED_ICLASS_POPAD,
  // XED_ICLASS_POPCNT,
  // XED_ICLASS_POPF,
  // XED_ICLASS_POPFD,
  // XED_ICLASS_POPFQ,
  case XED_ICLASS_POR:
    analyzeBinary(ins, Or, true);
    break;
  case XED_ICLASS_PREFETCHNTA:
  case XED_ICLASS_PREFETCHT0:
  case XED_ICLASS_PREFETCHT1:
  case XED_ICLASS_PREFETCHT2:
  case XED_ICLASS_PREFETCHW:
  case XED_ICLASS_PREFETCH_EXCLUSIVE:
  case XED_ICLASS_PREFETCH_RESERVED:
    // nothing to do for prefetch
    break;
  // XED_ICLASS_PSADBW,
  case XED_ICLASS_PSHUFB:
    analyzePshufb(ins);
    break;
  case XED_ICLASS_PSHUFD:
    analyzePshuf(ins, 4);
    break;
  case XED_ICLASS_PSHUFHW:
    analyzePshufw(ins, true);
    break;
  case XED_ICLASS_PSHUFLW:
    analyzePshufw(ins, false);
    break;
  case XED_ICLASS_PSHUFW:
    analyzePshuf(ins, 2);
    break;
  // XED_ICLASS_PSIGNB,
  // XED_ICLASS_PSIGND,
  // XED_ICLASS_PSIGNW,
  case XED_ICLASS_PSLLD:
    analyzePshift(ins, Shl, 4);
    break;
  case XED_ICLASS_PSLLDQ:
    analyzePshift(ins, Shl, 16);
    break;
  case XED_ICLASS_PSLLQ:
    analyzePshift(ins, Shl, 8);
    break;
  case XED_ICLASS_PSLLW:
    analyzePshift(ins, Shl, 2);
    break;
  case XED_ICLASS_PSRAD:
    analyzePshift(ins, AShr, 4);
    break;
  case XED_ICLASS_PSRAW:
    analyzePshift(ins, AShr, 2);
    break;
  case XED_ICLASS_PSRLD:
    analyzePshift(ins, LShr, 4);
    break;
  case XED_ICLASS_PSRLDQ:
    analyzePshift(ins, LShr, 16);
    break;
  case XED_ICLASS_PSRLQ:
    analyzePshift(ins, LShr, 8);
    break;
  case XED_ICLASS_PSRLW:
    analyzePshift(ins, LShr, 2);
    break;
  case XED_ICLASS_PSUBB:
    analyzeParith(ins, Sub, 1);
    break;
  case XED_ICLASS_PSUBD:
    analyzeParith(ins, Sub, 4);
    break;
  case XED_ICLASS_PSUBQ:
    analyzeParith(ins, Sub, 8);
    break;
  // XED_ICLASS_PSUBSB,
  // XED_ICLASS_PSUBSW,
  // XED_ICLASS_PSUBUSB,
  // XED_ICLASS_PSUBUSW,
  case XED_ICLASS_PSUBW:
    analyzeParith(ins, Sub, 2);
    break;
  // XED_ICLASS_PSWAPD,
  case XED_ICLASS_PTEST:
    // TODO: need to fix
    analyzeBinary(ins, And, false);
    break;
  case XED_ICLASS_PUNPCKHBW:
    analyzePunpckh(ins, 1);
    break;
  case XED_ICLASS_PUNPCKHDQ:
    analyzePunpckh(ins, 4);
    break;
  case XED_ICLASS_PUNPCKHQDQ:
    analyzePunpckh(ins, 8);
    break;
  case XED_ICLASS_PUNPCKHWD:
    analyzePunpckh(ins, 2);
    break;
  case XED_ICLASS_PUNPCKLBW:
    analyzePunpckl(ins, 1);
    break;
  case XED_ICLASS_PUNPCKLDQ:
    analyzePunpckl(ins, 4);
    break;
  case XED_ICLASS_PUNPCKLQDQ:
    analyzePunpckl(ins, 8);
    break;
  case XED_ICLASS_PUNPCKLWD:
    analyzePunpckl(ins, 2);
    break;
  case XED_ICLASS_PUSH:
    analyzePush(ins);
    break;
  // XED_ICLASS_PUSHA,
  // XED_ICLASS_PUSHAD,
  // XED_ICLASS_PUSHF,
  // XED_ICLASS_PUSHFD,
  // XED_ICLASS_PUSHFQ,
  case XED_ICLASS_PXOR:
    analyzeBinary(ins, Xor, true);
    break;
  // XED_ICLASS_RCL,
  // XED_ICLASS_RCPPS,
  // XED_ICLASS_RCPSS,
  // XED_ICLASS_RCR,
  // XED_ICLASS_RDFSBASE,
  // XED_ICLASS_RDGSBASE,
  // XED_ICLASS_RDMSR,
  // XED_ICLASS_RDPMC,
  // XED_ICLASS_RDRAND,
  // XED_ICLASS_RDSEED,
  case XED_ICLASS_RDTSC:
    analyzeRdtsc(ins);
    break;
  case XED_ICLASS_RDTSCP:
    analyzeRdtscp(ins);
    break;
  case XED_ICLASS_RET_FAR:
  case XED_ICLASS_RET_NEAR:
    analyzeRet(ins);
    break;
  case XED_ICLASS_ROL:
    analyzeShift(ins, Rol);
    break;
  case XED_ICLASS_ROR:
    analyzeShift(ins, Ror);
    break;
  // XED_ICLASS_RORX,
  // XED_ICLASS_ROUNDPD,
  // XED_ICLASS_ROUNDPS,
  // XED_ICLASS_ROUNDSD,
  // XED_ICLASS_ROUNDSS,
  // XED_ICLASS_RSM,
  // XED_ICLASS_RSQRTPS,
  // XED_ICLASS_RSQRTSS,
  // XED_ICLASS_SAHF,
  // XED_ICLASS_SALC,
  case XED_ICLASS_SAR:
    analyzeShift(ins, AShr);
    break;
  // XED_ICLASS_SARX,
  case XED_ICLASS_SBB:
    analyzeCarry(ins, Sub);
    break;
  case XED_ICLASS_SCASB:
    analyzeScas(ins, 1);
    break;
  case XED_ICLASS_SCASD:
    analyzeScas(ins, 4);
    break;
  case XED_ICLASS_SCASQ:
    analyzeScas(ins, 8);
    break;
  case XED_ICLASS_SCASW:
    analyzeScas(ins, 2);
    break;
  case XED_ICLASS_SETB:
    analyzeSet(ins, JCC_B, false);
    break;
  case XED_ICLASS_SETBE:
    analyzeSet(ins, JCC_BE, false);
    break;
  case XED_ICLASS_SETL:
    analyzeSet(ins, JCC_L, false);
    break;
  case XED_ICLASS_SETLE:
    analyzeSet(ins, JCC_LE, false);
    break;
  case XED_ICLASS_SETNB:
    analyzeSet(ins, JCC_B, true);
    break;
  case XED_ICLASS_SETNBE:
    analyzeSet(ins, JCC_BE, true);
    break;
  case XED_ICLASS_SETNL:
    analyzeSet(ins, JCC_L, true);
    break;
  case XED_ICLASS_SETNLE:
    analyzeSet(ins, JCC_LE, true);
    break;
  case XED_ICLASS_SETNO:
    analyzeSet(ins, JCC_O, true);
    break;
  case XED_ICLASS_SETNP:
    analyzeSet(ins, JCC_P, true);
    break;
  case XED_ICLASS_SETNS:
    analyzeSet(ins, JCC_S, true);
    break;
  case XED_ICLASS_SETNZ:
    analyzeSet(ins, JCC_Z, true);
    break;
  case XED_ICLASS_SETO:
    analyzeSet(ins, JCC_O, false);
    break;
  case XED_ICLASS_SETP:
    analyzeSet(ins, JCC_P, false);
    break;
  case XED_ICLASS_SETS:
    analyzeSet(ins, JCC_S, false);
    break;
  case XED_ICLASS_SETZ:
    analyzeSet(ins, JCC_Z, false);
    break;
  // XED_ICLASS_SFENCE,
  // XED_ICLASS_SGDT,
  // XED_ICLASS_SHA1MSG1,
  // XED_ICLASS_SHA1MSG2,
  // XED_ICLASS_SHA1NEXTE,
  // XED_ICLASS_SHA1RNDS4,
  // XED_ICLASS_SHA256MSG1,
  // XED_ICLASS_SHA256MSG2,
  // XED_ICLASS_SHA256RNDS2,
  case XED_ICLASS_SHL:
    analyzeShift(ins, Shl);
    break;
  case XED_ICLASS_SHLD:
    analyzeShiftd(ins, Shl);
    break;
  // XED_ICLASS_SHLX,
  case XED_ICLASS_SHR:
    analyzeShift(ins, LShr);
    break;
  case XED_ICLASS_SHRD:
    analyzeShiftd(ins, LShr);
    break;
  // XED_ICLASS_SHRX,
  // XED_ICLASS_SHUFPD,
  // XED_ICLASS_SHUFPS,
  // XED_ICLASS_SIDT,
  // XED_ICLASS_SKINIT,
  // XED_ICLASS_SLDT,
  // XED_ICLASS_SLWPCB,
  // XED_ICLASS_SMSW,
  // XED_ICLASS_SQRTPD,
  // XED_ICLASS_SQRTPS,
  // XED_ICLASS_SQRTSD,
  // XED_ICLASS_SQRTSS,
  // XED_ICLASS_STAC,
  // XED_ICLASS_STC,
  // XED_ICLASS_STD,
  // XED_ICLASS_STGI,
  // XED_ICLASS_STI,
  // XED_ICLASS_STMXCSR,
  case XED_ICLASS_STOSB:
    analyzeStos(ins);
    break;
  case XED_ICLASS_STOSD:
    analyzeStos(ins);
    break;
  case XED_ICLASS_STOSQ:
    analyzeStos(ins);
    break;
  case XED_ICLASS_STOSW:
    analyzeStos(ins);
    break;
  // XED_ICLASS_STR,
  case XED_ICLASS_SUB:
    analyzeBinary(ins, Sub, true);
    break;
  // XED_ICLASS_SUBPD,
  // XED_ICLASS_SUBPS,
  // XED_ICLASS_SUBSD,
  // XED_ICLASS_SUBSS,
  // XED_ICLASS_SWAPGS,
  // XED_ICLASS_SYSCALL,
  // XED_ICLASS_SYSCALL_AMD,
  // XED_ICLASS_SYSENTER,
  // XED_ICLASS_SYSEXIT,
  // XED_ICLASS_SYSRET,
  // XED_ICLASS_SYSRET_AMD,
  // XED_ICLASS_T1MSKC,
  case XED_ICLASS_TEST:
    analyzeBinary(ins, And, false);
    break;
  case XED_ICLASS_TZCNT:
    analyzeBs(ins, false);
    break;
  // XED_ICLASS_TZMSK,
  // XED_ICLASS_UCOMISD,
  // XED_ICLASS_UCOMISS,
  // XED_ICLASS_UD2,
  // XED_ICLASS_UNPCKHPD,
  // XED_ICLASS_UNPCKHPS,
  // XED_ICLASS_UNPCKLPD,
  // XED_ICLASS_UNPCKLPS,
  // XED_ICLASS_VADDPD,
  // XED_ICLASS_VADDPS,
  // XED_ICLASS_VADDSD,
  // XED_ICLASS_VADDSS,
  // XED_ICLASS_VADDSUBPD,
  // XED_ICLASS_VADDSUBPS,
  // XED_ICLASS_VAESDEC,
  // XED_ICLASS_VAESDECLAST,
  // XED_ICLASS_VAESENC,
  // XED_ICLASS_VAESENCLAST,
  // XED_ICLASS_VAESIMC,
  // XED_ICLASS_VAESKEYGENASSIST,
  // XED_ICLASS_VANDNPD,
  // XED_ICLASS_VANDNPS,
  // XED_ICLASS_VANDPD,
  // XED_ICLASS_VANDPS,
  // XED_ICLASS_VBLENDPD,
  // XED_ICLASS_VBLENDPS,
  // XED_ICLASS_VBLENDVPD,
  // XED_ICLASS_VBLENDVPS,
  // XED_ICLASS_VBROADCASTF128,
  // XED_ICLASS_VBROADCASTI128,
  // XED_ICLASS_VBROADCASTSD,
  // XED_ICLASS_VBROADCASTSS,
  // XED_ICLASS_VCMPPD,
  // XED_ICLASS_VCMPPS,
  // XED_ICLASS_VCMPSD,
  // XED_ICLASS_VCMPSS,
  // XED_ICLASS_VCOMISD,
  // XED_ICLASS_VCOMISS,
  // XED_ICLASS_VCVTDQ2PD,
  // XED_ICLASS_VCVTDQ2PS,
  // XED_ICLASS_VCVTPD2DQ,
  // XED_ICLASS_VCVTPD2PS,
  // XED_ICLASS_VCVTPH2PS,
  // XED_ICLASS_VCVTPS2DQ,
  // XED_ICLASS_VCVTPS2PD,
  // XED_ICLASS_VCVTPS2PH,
  // XED_ICLASS_VCVTSD2SI,
  // XED_ICLASS_VCVTSD2SS,
  // XED_ICLASS_VCVTSI2SD,
  // XED_ICLASS_VCVTSI2SS,
  // XED_ICLASS_VCVTSS2SD,
  // XED_ICLASS_VCVTSS2SI,
  // XED_ICLASS_VCVTTPD2DQ,
  // XED_ICLASS_VCVTTPS2DQ,
  // XED_ICLASS_VCVTTSD2SI,
  // XED_ICLASS_VCVTTSS2SI,
  // XED_ICLASS_VDIVPD,
  // XED_ICLASS_VDIVPS,
  // XED_ICLASS_VDIVSD,
  // XED_ICLASS_VDIVSS,
  // XED_ICLASS_VDPPD,
  // XED_ICLASS_VDPPS,
  // XED_ICLASS_VERR,
  // XED_ICLASS_VERW,
  // XED_ICLASS_VEXTRACTF128,
  // XED_ICLASS_VEXTRACTI128,
  // XED_ICLASS_VEXTRACTPS,
  //XED_ICLASS_VFMADD132PD,
  //XED_ICLASS_VFMADD132PS,
  //XED_ICLASS_VFMADD132SD,
  //XED_ICLASS_VFMADD132SS,
  //XED_ICLASS_VFMADD213PD,
  //XED_ICLASS_VFMADD213PS,
  //XED_ICLASS_VFMADD213SD,
  //XED_ICLASS_VFMADD213SS,
  //XED_ICLASS_VFMADD231PD,
  //XED_ICLASS_VFMADD231PS,
  //XED_ICLASS_VFMADD231SD,
  //XED_ICLASS_VFMADD231SS,
  //XED_ICLASS_VFMADDPD,
  //XED_ICLASS_VFMADDPS,
  //XED_ICLASS_VFMADDSD,
  //XED_ICLASS_VFMADDSS,
  //XED_ICLASS_VFMADDSUB132PD,
  //XED_ICLASS_VFMADDSUB132PS,
  //XED_ICLASS_VFMADDSUB213PD,
  //XED_ICLASS_VFMADDSUB213PS,
  //XED_ICLASS_VFMADDSUB231PD,
  //XED_ICLASS_VFMADDSUB231PS,
  //XED_ICLASS_VFMADDSUBPD,
  //XED_ICLASS_VFMADDSUBPS,
  //XED_ICLASS_VFMSUB132PD,
  //XED_ICLASS_VFMSUB132PS,
  //XED_ICLASS_VFMSUB132SD,
  //XED_ICLASS_VFMSUB132SS,
  //XED_ICLASS_VFMSUB213PD,
  //XED_ICLASS_VFMSUB213PS,
  //XED_ICLASS_VFMSUB213SD,
  //XED_ICLASS_VFMSUB213SS,
  //XED_ICLASS_VFMSUB231PD,
  //XED_ICLASS_VFMSUB231PS,
  //XED_ICLASS_VFMSUB231SD,
  //XED_ICLASS_VFMSUB231SS,
  //XED_ICLASS_VFMSUBADD132PD,
  //XED_ICLASS_VFMSUBADD132PS,
  //XED_ICLASS_VFMSUBADD213PD,
  //XED_ICLASS_VFMSUBADD213PS,
  //XED_ICLASS_VFMSUBADD231PD,
  //XED_ICLASS_VFMSUBADD231PS,
  //XED_ICLASS_VFMSUBADDPD,
  //XED_ICLASS_VFMSUBADDPS,
  //XED_ICLASS_VFMSUBPD,
  //XED_ICLASS_VFMSUBPS,
  //XED_ICLASS_VFMSUBSD,
  //XED_ICLASS_VFMSUBSS,
  //XED_ICLASS_VFNMADD132PD,
  //XED_ICLASS_VFNMADD132PS,
  //XED_ICLASS_VFNMADD132SD,
  //XED_ICLASS_VFNMADD132SS,
  //XED_ICLASS_VFNMADD213PD,
  //XED_ICLASS_VFNMADD213PS,
  //XED_ICLASS_VFNMADD213SD,
  //XED_ICLASS_VFNMADD213SS,
  //XED_ICLASS_VFNMADD231PD,
  //XED_ICLASS_VFNMADD231PS,
  //XED_ICLASS_VFNMADD231SD,
  //XED_ICLASS_VFNMADD231SS,
  //XED_ICLASS_VFNMADDPD,
  //XED_ICLASS_VFNMADDPS,
  //XED_ICLASS_VFNMADDSD,
  //XED_ICLASS_VFNMADDSS,
  //XED_ICLASS_VFNMSUB132PD,
  //XED_ICLASS_VFNMSUB132PS,
  //XED_ICLASS_VFNMSUB132SD,
  //XED_ICLASS_VFNMSUB132SS,
  //XED_ICLASS_VFNMSUB213PD,
  //XED_ICLASS_VFNMSUB213PS,
  //XED_ICLASS_VFNMSUB213SD,
  //XED_ICLASS_VFNMSUB213SS,
  //XED_ICLASS_VFNMSUB231PD,
  //XED_ICLASS_VFNMSUB231PS,
  //XED_ICLASS_VFNMSUB231SD,
  //XED_ICLASS_VFNMSUB231SS,
  //XED_ICLASS_VFNMSUBPD,
  //XED_ICLASS_VFNMSUBPS,
  //XED_ICLASS_VFNMSUBSD,
  //XED_ICLASS_VFNMSUBSS,
  //XED_ICLASS_VFRCZPD,
  //XED_ICLASS_VFRCZPS,
  //XED_ICLASS_VFRCZSD,
  //XED_ICLASS_VFRCZSS,
  //XED_ICLASS_VGATHERDPD,
  //XED_ICLASS_VGATHERDPS,
  //XED_ICLASS_VGATHERQPD,
  //XED_ICLASS_VGATHERQPS,
  //XED_ICLASS_VHADDPD,
  //XED_ICLASS_VHADDPS,
  //XED_ICLASS_VHSUBPD,
  //XED_ICLASS_VHSUBPS,
  case XED_ICLASS_VINSERTF128:
  case XED_ICLASS_VINSERTI128:
    analyzeVinserti128(ins);
    break;
  // XED_ICLASS_VINSERTPS,
  // XED_ICLASS_VLDDQU,
  // XED_ICLASS_VLDMXCSR,
  // XED_ICLASS_VMASKMOVDQU,
  // XED_ICLASS_VMASKMOVPD,
  // XED_ICLASS_VMASKMOVPS,
  // XED_ICLASS_VMAXPD,
  // XED_ICLASS_VMAXPS,
  // XED_ICLASS_VMAXSD,
  // XED_ICLASS_VMAXSS,
  // XED_ICLASS_VMCALL,
  // XED_ICLASS_VMCLEAR,
  // XED_ICLASS_VMFUNC,
  // XED_ICLASS_VMINPD,
  // XED_ICLASS_VMINPS,
  // XED_ICLASS_VMINSD,
  // XED_ICLASS_VMINSS,
  // XED_ICLASS_VMLAUNCH,
  // XED_ICLASS_VMLOAD,
  // XED_ICLASS_VMMCALL,
  case XED_ICLASS_VMOVAPD:
  case XED_ICLASS_VMOVAPS:
    analyzeMov(ins);
    break;
  case XED_ICLASS_VMOVD:
    analyzeMovlz(ins, 4);
    break;
  // XED_ICLASS_VMOVDDUP,
  case XED_ICLASS_VMOVDQA:
    analyzeMov(ins);
    break;
  case XED_ICLASS_VMOVDQU:
    analyzeMov(ins);
    break;
  // XED_ICLASS_VMOVHLPS,
  // XED_ICLASS_VMOVHPD,
  // XED_ICLASS_VMOVHPS,
  // XED_ICLASS_VMOVLHPS,
  case XED_ICLASS_VMOVLPD:
  case XED_ICLASS_VMOVLPS:
    analyzeVmovl(ins);
    break;
  // XED_ICLASS_VMOVMSKPD,
  // XED_ICLASS_VMOVMSKPS,
  case XED_ICLASS_VMOVNTDQ:
  case XED_ICLASS_VMOVNTDQA:
    analyzeMov(ins);
    break;
  // XED_ICLASS_VMOVNTPD,
  // XED_ICLASS_VMOVNTPS,
  case XED_ICLASS_VMOVQ:
    analyzeMovlz(ins, 8);
    break;
  // XED_ICLASS_VMOVSD,
  // XED_ICLASS_VMOVSHDUP,
  // XED_ICLASS_VMOVSLDUP,
  // XED_ICLASS_VMOVSS,
  // XED_ICLASS_VMOVUPD,
  // XED_ICLASS_VMOVUPS,
  // XED_ICLASS_VMPSADBW,
  // XED_ICLASS_VMPTRLD,
  // XED_ICLASS_VMPTRST,
  // XED_ICLASS_VMREAD,
  // XED_ICLASS_VMRESUME,
  // XED_ICLASS_VMRUN,
  // XED_ICLASS_VMSAVE,
  // XED_ICLASS_VMULPD,
  // XED_ICLASS_VMULPS,
  // XED_ICLASS_VMULSD,
  // XED_ICLASS_VMULSS,
  // XED_ICLASS_VMWRITE,
  // XED_ICLASS_VMXOFF,
  // XED_ICLASS_VMXON,
  // XED_ICLASS_VORPD,
  // XED_ICLASS_VORPS,
  // XED_ICLASS_VPABSB,
  // XED_ICLASS_VPABSD,
  // XED_ICLASS_VPABSW,
  case XED_ICLASS_VPACKSSDW:
    analyzeVpack(ins, 2, true);
    break;
  case XED_ICLASS_VPACKSSWB:
    analyzeVpack(ins, 1, true);
    break;
  case XED_ICLASS_VPACKUSDW:
    analyzeVpack(ins, 2, false);
    break;
  case XED_ICLASS_VPACKUSWB:
    analyzeVpack(ins, 1, false);
    break;
  case XED_ICLASS_VPADDB:
    analyzeVparith(ins, Add, 1);
    break;
  case XED_ICLASS_VPADDD:
    analyzeVparith(ins, Add, 4);
    break;
  case XED_ICLASS_VPADDQ:
    analyzeVparith(ins, Add, 8);
    break;
  // XED_ICLASS_VPADDSB,
  // XED_ICLASS_VPADDSW,
  // XED_ICLASS_VPADDUSB,
  // XED_ICLASS_VPADDUSW,
  case XED_ICLASS_VPADDW:
    analyzeVparith(ins, Add, 2);
    break;
  // XED_ICLASS_VPALIGNR,
  case XED_ICLASS_VPAND:
    analyzeBinary(ins, And, true);
    break;
  // XED_ICLASS_VPANDN,
  // XED_ICLASS_VPAVGB,
  // XED_ICLASS_VPAVGW,
  // XED_ICLASS_VPBLENDD,
  // XED_ICLASS_VPBLENDVB,
  // XED_ICLASS_VPBLENDW,
  // XED_ICLASS_VPBROADCASTB,
  // XED_ICLASS_VPBROADCASTD,
  // XED_ICLASS_VPBROADCASTQ,
  // XED_ICLASS_VPBROADCASTW,
  // XED_ICLASS_VPCLMULQDQ,
  // XED_ICLASS_VPCMOV,
  case XED_ICLASS_VPCMPEQB:
    analyzeVpcmp(ins, Equal, 1);
    break;
  case XED_ICLASS_VPCMPEQD:
    analyzeVpcmp(ins, Equal, 4);
    break;
  case XED_ICLASS_VPCMPEQQ:
    analyzeVpcmp(ins, Equal, 8);
    break;
  case XED_ICLASS_VPCMPEQW:
    analyzeVpcmp(ins, Equal, 2);
    break;
  // XED_ICLASS_VPCMPESTRI,
  // XED_ICLASS_VPCMPESTRM,
  case XED_ICLASS_VPCMPGTB:
    analyzeVpcmp(ins, Sgt, 1);
    break;
  case XED_ICLASS_VPCMPGTD:
    analyzeVpcmp(ins, Sgt, 4);
    break;
  case XED_ICLASS_VPCMPGTQ:
    analyzeVpcmp(ins, Sgt, 8);
    break;
  case XED_ICLASS_VPCMPGTW:
    analyzeVpcmp(ins, Sgt, 2);
    break;
  // XED_ICLASS_VPCMPISTRI,
  // XED_ICLASS_VPCMPISTRM,
  // XED_ICLASS_VPCOMB,
  // XED_ICLASS_VPCOMD,
  // XED_ICLASS_VPCOMQ,
  // XED_ICLASS_VPCOMUB,
  // XED_ICLASS_VPCOMUD,
  // XED_ICLASS_VPCOMUQ,
  // XED_ICLASS_VPCOMUW,
  // XED_ICLASS_VPCOMW,
  // XED_ICLASS_VPERM2F128,
  // XED_ICLASS_VPERM2I128,
  // XED_ICLASS_VPERMD,
  // XED_ICLASS_VPERMIL2PD,
  // XED_ICLASS_VPERMIL2PS,
  // XED_ICLASS_VPERMILPD,
  // XED_ICLASS_VPERMILPS,
  // XED_ICLASS_VPERMPD,
  // XED_ICLASS_VPERMPS,
  // XED_ICLASS_VPERMQ,
  case XED_ICLASS_VPEXTRB:
    analyzePextr(ins, 1);
    break;
  case XED_ICLASS_VPEXTRD:
    analyzePextr(ins, 4);
    break;
  case XED_ICLASS_VPEXTRQ:
    analyzePextr(ins, 8);
    break;
  case XED_ICLASS_VPEXTRW:
    analyzePextr(ins, 2);
    break;
  // XED_ICLASS_VPGATHERDD,
  // XED_ICLASS_VPGATHERDQ,
  // XED_ICLASS_VPGATHERQD,
  // XED_ICLASS_VPGATHERQQ,
  // XED_ICLASS_VPHADDBD,
  // XED_ICLASS_VPHADDBQ,
  // XED_ICLASS_VPHADDBW,
  // XED_ICLASS_VPHADDD,
  // XED_ICLASS_VPHADDDQ,
  // XED_ICLASS_VPHADDSW,
  // XED_ICLASS_VPHADDUBD,
  // XED_ICLASS_VPHADDUBQ,
  // XED_ICLASS_VPHADDUBW,
  // XED_ICLASS_VPHADDUDQ,
  // XED_ICLASS_VPHADDUWD,
  // XED_ICLASS_VPHADDUWQ,
  // XED_ICLASS_VPHADDW,
  // XED_ICLASS_VPHADDWD,
  // XED_ICLASS_VPHADDWQ,
  // XED_ICLASS_VPHMINPOSUW,
  // XED_ICLASS_VPHSUBBW,
  // XED_ICLASS_VPHSUBD,
  // XED_ICLASS_VPHSUBDQ,
  // XED_ICLASS_VPHSUBSW,
  // XED_ICLASS_VPHSUBW,
  // XED_ICLASS_VPHSUBWD,
  // XED_ICLASS_VPINSRB,
  // XED_ICLASS_VPINSRD,
  // XED_ICLASS_VPINSRQ,
  // XED_ICLASS_VPINSRW,
  // XED_ICLASS_VPMACSDD,
  // XED_ICLASS_VPMACSDQH,
  // XED_ICLASS_VPMACSDQL,
  // XED_ICLASS_VPMACSSDD,
  // XED_ICLASS_VPMACSSDQH,
  // XED_ICLASS_VPMACSSDQL,
  // XED_ICLASS_VPMACSSWD,
  // XED_ICLASS_VPMACSSWW,
  // XED_ICLASS_VPMACSWD,
  // XED_ICLASS_VPMACSWW,
  // XED_ICLASS_VPMADCSSWD,
  // XED_ICLASS_VPMADCSWD,
  // XED_ICLASS_VPMADDUBSW,
  case XED_ICLASS_VPMADDWD:
    analyzeVpmaddwd(ins);
    break;
  // XED_ICLASS_VPMASKMOVD,
  // XED_ICLASS_VPMASKMOVQ,
  case XED_ICLASS_VPMAXSB:
    analyzeVpcmpxchg(ins, Sgt, 1);
    break;
  case XED_ICLASS_VPMAXSD:
    analyzeVpcmpxchg(ins, Sgt, 4);
    break;
  case XED_ICLASS_VPMAXSW:
    analyzeVpcmpxchg(ins, Ugt, 2);
    break;
  case XED_ICLASS_VPMAXUB:
    analyzeVpcmpxchg(ins, Ugt, 1);
    break;
  case XED_ICLASS_VPMAXUD:
    analyzeVpcmpxchg(ins, Ugt, 4);
    break;
  case XED_ICLASS_VPMAXUW:
    analyzeVpcmpxchg(ins, Ugt, 2);
    break;
  case XED_ICLASS_VPMINSB:
    analyzeVpcmpxchg(ins, Slt, 1);
    break;
  case XED_ICLASS_VPMINSD:
    analyzeVpcmpxchg(ins, Slt, 4);
    break;
  case XED_ICLASS_VPMINSW:
    analyzeVpcmpxchg(ins, Slt, 2);
    break;
  case XED_ICLASS_VPMINUB:
    analyzeVpcmpxchg(ins, Ult, 1);
    break;
  case XED_ICLASS_VPMINUD:
    analyzeVpcmpxchg(ins, Ult, 4);
    break;
  case XED_ICLASS_VPMINUW:
    analyzeVpcmpxchg(ins, Ult, 2);
    break;
  // XED_ICLASS_VPMOVMSKB,
  // XED_ICLASS_VPMOVSXBD,
  // XED_ICLASS_VPMOVSXBQ,
  // XED_ICLASS_VPMOVSXBW,
  // XED_ICLASS_VPMOVSXDQ,
  // XED_ICLASS_VPMOVSXWD,
  // XED_ICLASS_VPMOVSXWQ,
  // XED_ICLASS_VPMOVZXBD,
  // XED_ICLASS_VPMOVZXBQ,
  // XED_ICLASS_VPMOVZXBW,
  // XED_ICLASS_VPMOVZXDQ,
  // XED_ICLASS_VPMOVZXWD,
  // XED_ICLASS_VPMOVZXWQ,
  // XED_ICLASS_VPMULDQ,
  // XED_ICLASS_VPMULHRSW,
  case XED_ICLASS_VPMULHUW:
    analyzeVpmulhw(ins, false);
    break;
  case XED_ICLASS_VPMULHW:
    analyzeVpmulhw(ins, true);
    break;
  case XED_ICLASS_VPMULLD:
    analyzeVpmul(ins, 4);
    break;
  case XED_ICLASS_VPMULLW:
    analyzeVpmul(ins, 2);
    break;
  // XED_ICLASS_VPMULUDQ,
  case XED_ICLASS_VPOR:
    analyzeTernary(ins, Or);
    break;
  // XED_ICLASS_VPPERM,
  // XED_ICLASS_VPROTB,
  // XED_ICLASS_VPROTD,
  // XED_ICLASS_VPROTQ,
  // XED_ICLASS_VPROTW,
  // XED_ICLASS_VPSADBW,
  // XED_ICLASS_VPSHAB,
  // XED_ICLASS_VPSHAD,
  // XED_ICLASS_VPSHAQ,
  // XED_ICLASS_VPSHAW,
  // XED_ICLASS_VPSHLB,
  // XED_ICLASS_VPSHLD,
  // XED_ICLASS_VPSHLQ,
  // XED_ICLASS_VPSHLW,
  case XED_ICLASS_VPSHUFB:
    analyzeVpshufb(ins);
    break;
  // XED_ICLASS_VPSHUFD,
  case XED_ICLASS_VPSHUFHW:
    analyzePshufw(ins, true);
    break;
  case XED_ICLASS_VPSHUFLW:
    analyzePshufw(ins, false);
    break;
  // XED_ICLASS_VPSIGNB,
  // XED_ICLASS_VPSIGND,
  // XED_ICLASS_VPSIGNW,
  case XED_ICLASS_VPSLLD:
    analyzeVpshift(ins, Shl, 4);
    break;
  case XED_ICLASS_VPSLLDQ:
    analyzeVpshift(ins, Shl, 16);
    break;
  case XED_ICLASS_VPSLLQ:
    analyzeVpshift(ins, Shl, 8);
    break;
  // XED_ICLASS_VPSLLVD,
  //  XED_ICLASS_VPSLLVQ,
  case XED_ICLASS_VPSLLW:
    analyzeVpshift(ins, Shl, 2);
    break;
  case XED_ICLASS_VPSRAD:
    analyzeVpshift(ins, AShr, 4);
    break;
  // XED_ICLASS_VPSRAVD,
  case XED_ICLASS_VPSRAW:
    analyzeVpshift(ins, AShr, 2);
    break;
  case XED_ICLASS_VPSRLD:
    analyzeVpshift(ins, LShr, 4);
    break;
  case XED_ICLASS_VPSRLDQ:
    analyzeVpshift(ins, LShr, 16);
    break;
  case XED_ICLASS_VPSRLQ:
    analyzeVpshift(ins, LShr, 8);
    break;
  // XED_ICLASS_VPSRLVD,
  // XED_ICLASS_VPSRLVQ,
  case XED_ICLASS_VPSRLW:
    analyzeVpshift(ins, LShr, 2);
    break;
  case XED_ICLASS_VPSUBB:
    analyzeVparith(ins, Sub, 1);
    break;
  case XED_ICLASS_VPSUBD:
    analyzeVparith(ins, Sub, 4);
    break;
  case XED_ICLASS_VPSUBQ:
    analyzeVparith(ins, Sub, 8);
    break;
  // XED_ICLASS_VPSUBSB,
  // XED_ICLASS_VPSUBSW,
  // XED_ICLASS_VPSUBUSB,
  // XED_ICLASS_VPSUBUSW,
  case XED_ICLASS_VPSUBW:
    analyzeVparith(ins, Sub, 2);
    break;
  case XED_ICLASS_VPTEST:
    // TODO: need to fix
    analyzeBinary(ins, And, false);
    break;
  case XED_ICLASS_VPUNPCKHBW:
    analyzePunpckh(ins, 1);
    break;
  case XED_ICLASS_VPUNPCKHDQ:
    analyzePunpckh(ins, 4);
    break;
  case XED_ICLASS_VPUNPCKHQDQ:
    analyzePunpckh(ins, 8);
    break;
  case XED_ICLASS_VPUNPCKHWD:
    analyzePunpckh(ins, 2);
    break;
  case XED_ICLASS_VPUNPCKLBW:
    analyzePunpckl(ins, 1);
    break;
  case XED_ICLASS_VPUNPCKLDQ:
    analyzePunpckl(ins, 4);
    break;
  case XED_ICLASS_VPUNPCKLQDQ:
    analyzePunpckl(ins, 8);
    break;
  case XED_ICLASS_VPUNPCKLWD:
    analyzePunpckl(ins, 2);
    break;
  case XED_ICLASS_VPXOR:
    analyzeTernary(ins, Xor);
    break;
  // XED_ICLASS_VRCPPS,
  // XED_ICLASS_VRCPSS,
  // XED_ICLASS_VROUNDPD,
  // XED_ICLASS_VROUNDPS,
  // XED_ICLASS_VROUNDSD,
  // XED_ICLASS_VROUNDSS,
  // XED_ICLASS_VRSQRTPS,
  // XED_ICLASS_VRSQRTSS,
  // XED_ICLASS_VSHUFPD,
  // XED_ICLASS_VSHUFPS,
  // XED_ICLASS_VSQRTPD,
  // XED_ICLASS_VSQRTPS,
  // XED_ICLASS_VSQRTSD,
  // XED_ICLASS_VSQRTSS,
  // XED_ICLASS_VSTMXCSR,
  // XED_ICLASS_VSUBPD,
  // XED_ICLASS_VSUBPS,
  // XED_ICLASS_VSUBSD,
  // XED_ICLASS_VSUBSS,
  // XED_ICLASS_VTESTPD,
  // XED_ICLASS_VTESTPS,
  // XED_ICLASS_VUCOMISD,
  // XED_ICLASS_VUCOMISS,
  // XED_ICLASS_VUNPCKHPD,
  // XED_ICLASS_VUNPCKHPS,
  // XED_ICLASS_VUNPCKLPD,
  // XED_ICLASS_VUNPCKLPS,
  // XED_ICLASS_VXORPD,
  case XED_ICLASS_VXORPS:
    analyzeTernary(ins, Xor);
    break;
  // XED_ICLASS_VZEROALL,
  // XED_ICLASS_VZEROUPPER,
  // XED_ICLASS_WBINVD,
  // XED_ICLASS_WRFSBASE,
  // XED_ICLASS_WRGSBASE,
  // XED_ICLASS_WRMSR,
  // XED_ICLASS_XABORT,
  case XED_ICLASS_XADD:
    analyzeXadd(ins);
    break;
  // XED_ICLASS_XBEGIN,
  case XED_ICLASS_XCHG:
    analyzeXchg(ins);
    break;
  // XED_ICLASS_XEND,
  // XED_ICLASS_XGETBV,
  // XED_ICLASS_XLAT,
  case XED_ICLASS_XOR:
    analyzeBinary(ins, Xor, true);
    break;
  case XED_ICLASS_XORPD:
    analyzeBinary(ins, Xor, true);
    break;
  case XED_ICLASS_XORPS:
    analyzeBinary(ins, Xor, true);
    break;
  // XED_ICLASS_XRSTOR,
  // XED_ICLASS_XRSTOR64,
  // XED_ICLASS_XRSTORS,
  // XED_ICLASS_XRSTORS64,
  // XED_ICLASS_XSAVE,
  // XED_ICLASS_XSAVE64,
  // XED_ICLASS_XSAVEC,
  // XED_ICLASS_XSAVEC64,
  // XED_ICLASS_XSAVEOPT,
  // XED_ICLASS_XSAVEOPT64,
  // XED_ICLASS_XSAVES,
  // XED_ICLASS_XSAVES64,
  // XED_ICLASS_XSETBV,
  // XED_ICLASS_XTEST,
  default:
    analyzeDefault(ins);
    LOG_DEBUG("Missing instruction: " + INS_Disassemble(ins) + "\n");
    break;
  }
  return;
}

} // namespace qsym
