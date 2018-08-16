#ifndef QSYM_SYSCALL_HOOK_
#define QSYM_SYSCALL_HOOK_

#include <string>
#include "syscall_desc.h"
#include "linux_syscall.h"

namespace qsym {

void hookSyscalls(bool hook_stdin, bool hook_fs, bool hook_net, 
                  const std::string& input);

} // namespace qsym;

#endif // QSYM_SYSCALL_HOOK_
