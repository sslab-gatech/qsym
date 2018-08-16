#include "api.h"
#include "analysis.h"
#include "memory.h"
#include "solver.h"
#include "thread_context.h"
#include "syscall_context.h"

#define CLEAR_EFLAGS_AC(eflags)	((eflags & 0xfffbffff))

namespace qsym {

extern Memory       g_memory;
extern REG          g_thread_context_reg;
extern Solver        *g_solver;


bool kDynLdLnkLoaded = false;

static void
loadImage(IMG img, VOID* v) {
  LOG_INFO("IMG: " + IMG_Name(img) + "\n");
  if (kDynLdLnkLoaded)
    return;

  g_memory.mmap(IMG_LowAddress(img), IMG_HighAddress(img));
  g_memory.initializeBrk(IMG_HighAddress(img));

	if (IMG_Name(img).compare("/lib/ld-linux.so.2") == 0 ||
			IMG_Type(img) == IMG_TYPE_STATIC)
    kDynLdLnkLoaded = true;
}

// from libdft
// global handler for internal errors (i.e., errors from qsym)
// handle memory protection (e.g., R/W/X access to null_seg)
// -- or --
// for unknown reasons, when an analysis function is executed,
// the EFLAGS.AC bit (i.e., bit 18) is asserted, thus leading
// into a runtime exception whenever an unaligned read/write
// is performed from qsym. This callback can be registered
// with PIN_AddInternalexceptionHandler() so as to trap the
// generated signal and remediate
static EXCEPT_HANDLING_RESULT
exceptionHandler(THREADID tid, EXCEPTION_INFO *pExceptInfo,
		PHYSICAL_CONTEXT *pPhysCtxt, VOID *v) {
	ADDRINT vaddr = 0x0;

	// unaligned memory accesses
	if (PIN_GetExceptionCode(pExceptInfo) ==
			EXCEPTCODE_ACCESS_MISALIGNED) {
		// clear EFLAGS.AC
		PIN_SetPhysicalContextReg(pPhysCtxt, REG_EFLAGS,
			CLEAR_EFLAGS_AC(PIN_GetPhysicalContextReg(pPhysCtxt,
					REG_EFLAGS)));

		// the exception is handled gracefully
		return EHR_HANDLED;
	}
	// memory protection
	else if (PIN_GetExceptionCode(pExceptInfo) ==
			EXCEPTCODE_ACCESS_DENIED) {

		// get the address of the memory violation
		PIN_GetFaultyAccessAddress(pExceptInfo, &vaddr);

    if (g_memory.isUnmappedAddress(vaddr)) {
      std::cerr << "invalid access -- memory protection triggered\n";
      exit(0);
    }
	}
	return EHR_UNHANDLED;
}

static inline void
allocateThreadContext(THREADID tid, CONTEXT* ctx, INT32 flags, VOID* v) {
  g_memory.allocateStack(PIN_GetContextReg(ctx, REG_STACK_PTR));
  ThreadContext* thread_ctx = new ThreadContext();
  PIN_SetContextReg(ctx, g_thread_context_reg, (ADDRINT)thread_ctx);
}

static inline void
freeThreadContext(THREADID tid, const CONTEXT* ctx, INT32 code, VOID* v) {
  ThreadContext* thread_ctx =
    reinterpret_cast<ThreadContext*>(PIN_GetContextReg(ctx, g_thread_context_reg));
  delete thread_ctx;
}

static inline void
initializeThreadContext() {
	if ((g_thread_context_reg = PIN_ClaimToolRegister()) == REG_INVALID())
    LOG_FATAL("register claim failed\n");

	PIN_AddThreadStartFunction(allocateThreadContext, NULL);
	PIN_AddThreadFiniFunction(freeThreadContext,	NULL);
}

static inline void
initializeMemory() {
  g_memory.initialize();
	IMG_AddInstrumentFunction(loadImage, NULL);
}

static void
onSyscallEnter(THREADID tid, CONTEXT* ctx, SYSCALL_STANDARD std, VOID* v) {
  ThreadContext* thread_ctx = reinterpret_cast<ThreadContext*>(
      PIN_GetContextReg(ctx, g_thread_context_reg));
  thread_ctx->onSyscallEnter(ctx, std);
}

static void
onSyscallExit(THREADID tid, CONTEXT* ctx, SYSCALL_STANDARD std, VOID* v) {
  ThreadContext* thread_ctx = reinterpret_cast<ThreadContext*>(
      PIN_GetContextReg(ctx, g_thread_context_reg));
  thread_ctx->onSyscallExit(ctx, std);
  thread_ctx->clearExprFromReg(getAx(sizeof(ADDRINT)));
}

void initializeQsym() {
  initializeThreadContext();
  initializeMemory();

	PIN_AddSyscallEntryFunction(onSyscallEnter, NULL);
	PIN_AddSyscallExitFunction(onSyscallExit, NULL);
	TRACE_AddInstrumentFunction(analyzeTrace, NULL);
	PIN_AddInternalExceptionHandler(exceptionHandler, NULL);
}

} // namespace
