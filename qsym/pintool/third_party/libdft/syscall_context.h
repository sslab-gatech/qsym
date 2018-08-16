#ifndef QSYM_SYSCALL_CONTEXT_H_
#define QSYM_SYSCALL_CONTEXT_H_

#include <sys/syscall.h>
#include <linux/version.h>
#include "pin.H"

namespace qsym {

#if __x86_64__
  const INT32 kSyscallMax = __NR_seccomp + 1;
#else
  const INT32 kSyscallMax = __NR_syncfs + 1;
#endif

enum {
  SYSCALL_ARG0 = 0,
  SYSCALL_ARG1 = 1,
  SYSCALL_ARG2 = 2,
  SYSCALL_ARG3 = 3,
  SYSCALL_ARG4 = 4,
  SYSCALL_ARG5 = 5,
};

const INT32 kMaxSyscallArgNum = SYSCALL_ARG5 + 1;

typedef struct {
    int nr;
    ADDRINT arg[kMaxSyscallArgNum];
    ADDRINT ret;
    void* aux;
} SyscallContext;

void
postMMapHook(SyscallContext *ctx);

} // namespace qsym
#endif // QSYM_SYSCALL_CONTEXT_H_
