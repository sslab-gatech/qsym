#ifndef QSYM_SYSCALL_DESC_H_
#define QSYM_SYSCALL_DESC_H_

#include <sys/syscall.h>
#include <linux/version.h>
#include "syscall_context.h"
#include "compiler.h"

namespace qsym {

typedef struct {
	size_t	nargs;
	size_t	save_args;
  size_t  retval_args;
	size_t	map_args[kMaxSyscallArgNum];
	void	(* pre)(SyscallContext*);
	void	(* post)(SyscallContext*);
} SyscallDesc;


void initializeSyscallDesc();
int setSyscallPre(SyscallDesc *desc, void (* pre)(SyscallContext*));
int setSyscallPost(SyscallDesc *desc, void (* post)(SyscallContext*));
int clearSyscallPre(SyscallDesc *desc);
int clearSyscallPost(SyscallDesc *desc);

} // namespace qsym
#endif // QSYM_SYSCALL_DESC_H_
