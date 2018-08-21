#include <sys/epoll.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <asm/fcntl.h>
#include <asm/stat.h>

#include <poll.h>
#include <string.h>
#include <unistd.h>

#include <map>

#include <linux/mempolicy.h>
#include <linux/sysctl.h>

#include "memory.h"
#include "syscall_desc.h"
#include "linux_syscall.h"

namespace qsym {
// shared memory segments (address -> size)
std::map<ADDRINT, USIZE> kSharedMemoryMap;
extern Memory g_memory;

// callbacks declaration
static void postReadHook(SyscallContext*);
static void postUseLibHook(SyscallContext*);
static void postBrkHook(SyscallContext*);
static void postFcntlHook(SyscallContext*);
static void postGetGroups16Hook(SyscallContext*);
// static void postMMapHook(SyscallContext*);
static void postMUnmapHook(SyscallContext*);
static void postSocketCallHook(SyscallContext*);
static void postSyslogHook(SyscallContext*);
static void postIpcHook(SyscallContext*);
static void postModifyLdtHook(SyscallContext*);
static void postQuotatlHook(SyscallContext *ctx);
static void postReadvHook(SyscallContext*);
static void postSysctlHook(SyscallContext*);
static void postMRemapHook(SyscallContext*);
static void postPollHook(SyscallContext *ctx);
static void postRtSigPendingHook(SyscallContext *ctx);
static void postGetCwdHook(SyscallContext *ctx);
static void postGetGroupsHook(SyscallContext*);
static void postMincoreHook(SyscallContext *ctx);
static void postGetDentsHook(SyscallContext *ctx);
static void postGetxattrHook(SyscallContext *ctx);
static void postListXattrHook(SyscallContext *ctx);
static void postIoGetEventsHook(SyscallContext *ctx);
static void postGetMemPolicy(SyscallContext *ctx);
static void postLookupDcookieHook(SyscallContext *ctx);
static void postMqTimedReceiveHook(SyscallContext *ctx);
static void postReadLinkAtHook(SyscallContext*);
static void postEpollWaitHook(SyscallContext *ctx);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
static void postRecvMmsgHook(SyscallContext *ctx);
#endif

// syscall descriptors
SyscallDesc kSyscallDesc[kSyscallMax];

namespace {
inline void
postGetGroupsHookN(SyscallContext *ctx, USIZE struct_size) {
	if ((long)ctx->ret <= 0 || (old_gid_t *)ctx->arg[SYSCALL_ARG1] == NULL)
		return;

	// clear the tag bits
	g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG1],
			(struct_size * (size_t)ctx->ret));
}

} // namespace

void
initializeSyscallDesc() {
  kSyscallDesc[__NR_restart_syscall] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_exit] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_fork] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_read] = SyscallDesc{3, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postReadHook};
  kSyscallDesc[__NR_write] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_open] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_close] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_creat] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_link] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_unlink] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_execve] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_chdir] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_time] = SyscallDesc{1, 0, 1, {sizeof(time_t), 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_mknod] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_chmod] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_lchown] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_stat] = SyscallDesc{2, 0, 1, {0, sizeof(struct __old_kernel_stat), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_lseek] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getpid] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_mount] = SyscallDesc{5, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_setuid] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getuid] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};

  kSyscallDesc[__NR_ptrace] = SyscallDesc{4, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_alarm] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_fstat] = SyscallDesc{2, 0, 1, {0, sizeof(struct __old_kernel_stat), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_pause] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_utime] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_access] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sync] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_kill] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_rename] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_mkdir] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_rmdir] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_dup] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_pipe] = SyscallDesc{1, 0, 1, {sizeof(int)*2, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_times] = SyscallDesc{1, 0, 1, {sizeof(struct tms), 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_brk] = SyscallDesc{1, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postBrkHook};
  kSyscallDesc[__NR_setgid] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getgid] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_geteuid] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getegid] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_acct] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_umount2] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_ioctl] = SyscallDesc{3, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_fcntl] = SyscallDesc{3, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postFcntlHook};
  kSyscallDesc[__NR_setpgid] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_umask] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_chroot] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_ustat] = SyscallDesc{2, 0, 1, {0, sizeof(struct ustat), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_dup2] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getppid] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getpgrp] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_setsid] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_setreuid] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_setregid] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sethostname] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_setrlimit] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getrlimit] = SyscallDesc{2, 0, 1, {0, sizeof(struct rlimit), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getrusage] = SyscallDesc{2, 0, 1, {0, sizeof(struct rusage), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_gettimeofday] = SyscallDesc{2, 0, 1, {sizeof(struct timeval), sizeof(struct timezone), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_settimeofday] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getgroups] = SyscallDesc{2, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postGetGroups16Hook};
  kSyscallDesc[__NR_setgroups] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_select] = SyscallDesc{5, 0, 1, {0, sizeof(fd_set), sizeof(fd_set), sizeof(fd_set), sizeof(struct timeval), 0}, NULL, NULL};
  kSyscallDesc[__NR_symlink] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_lstat] = SyscallDesc{2, 0, 1, {0, sizeof(struct __old_kernel_stat), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_readlink] = SyscallDesc{3, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postReadHook};
  kSyscallDesc[__NR_uselib] = SyscallDesc{1, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postUseLibHook};
  kSyscallDesc[__NR_swapon] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_reboot] = SyscallDesc{4, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_mmap] = SyscallDesc{6, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postMMapHook};
  kSyscallDesc[__NR_munmap] = SyscallDesc{2, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postMUnmapHook};
  kSyscallDesc[__NR_truncate] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_ftruncate] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_fchmod] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_fchown] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getpriority] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_setpriority] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_statfs] = SyscallDesc{2, 0, 1, {0, sizeof(struct statfs), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_fstatfs] = SyscallDesc{2, 0, 1, {0, sizeof(struct statfs), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_ioperm] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_syslog] = SyscallDesc{3, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postSyslogHook};
  kSyscallDesc[__NR_setitimer] = SyscallDesc{3, 0, 1, {0, 0, sizeof(struct itimerval), 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getitimer] = SyscallDesc{2, 0, 1, {0, sizeof(struct itimerval), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_stat] = SyscallDesc{2, 0, 1, {0, sizeof(struct stat), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_lstat] = SyscallDesc{2, 0, 1, {0, sizeof(struct stat), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_fstat] = SyscallDesc{2, 0, 1, {0, sizeof(struct stat), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_uname] = SyscallDesc{1, 0, 1, {sizeof(struct new_utsname), 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_iopl] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_vhangup] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_wait4] = SyscallDesc{4, 0, 1, {0, sizeof(int), 0, sizeof(struct rusage), 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_swapoff] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sysinfo] = SyscallDesc{1, 0, 1, {sizeof(struct sysinfo), 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_fsync] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_clone] = SyscallDesc{5, 0, 1, {0, 0, sizeof(int), 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_setdomainname] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_uname] = SyscallDesc{1, 0, 1, {sizeof(struct new_utsname), 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_modify_ldt] = SyscallDesc{3, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postModifyLdtHook};
  kSyscallDesc[__NR_adjtimex] = SyscallDesc{1, 0, 1, {sizeof(struct timex), 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_mprotect] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_create_module] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_init_module] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_delete_module] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_get_kernel_syms] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_quotactl] = SyscallDesc{4, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postQuotatlHook};
  kSyscallDesc[__NR_getpgid] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_fchdir] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sysfs] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_personality] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_afs_syscall] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_setfsuid] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_setfsgid] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getdents] = SyscallDesc{3, 0, 1, {0, sizeof(struct linux_dirent), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_select] = SyscallDesc{5, 0, 1, {0, sizeof(fd_set), sizeof(fd_set), sizeof(fd_set), sizeof(struct timeval), 0}, NULL, NULL};
  kSyscallDesc[__NR_flock] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_msync] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_readv] = SyscallDesc{3, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postReadvHook};
  kSyscallDesc[__NR_writev] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getsid] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_fdatasync] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR__sysctl] = SyscallDesc{1, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postSysctlHook};
  kSyscallDesc[__NR_mlock] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_munlock] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_mlockall] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_munlockall] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sched_setparam] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sched_getparam] = SyscallDesc{2, 0, 1, {0, sizeof(struct sched_param), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sched_setscheduler] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sched_getscheduler] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sched_yield] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sched_get_priority_max] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sched_get_priority_min] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sched_rr_get_interval] = SyscallDesc{2, 0, 1, {0, sizeof(struct timespec), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_nanosleep] = SyscallDesc{2, 0, 1, {0, sizeof(struct timespec), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_mremap] = SyscallDesc{5, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postMRemapHook};
  kSyscallDesc[__NR_setresuid] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getresuid] = SyscallDesc{3, 0, 1, {sizeof(old_uid_t), sizeof(old_uid_t), sizeof(old_uid_t), 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_query_module] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_poll] = SyscallDesc{3, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postPollHook};
  kSyscallDesc[__NR_nfsservctl] = SyscallDesc{3, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_setresgid] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getresgid] = SyscallDesc{3, 0, 1, {sizeof(old_gid_t), sizeof(old_gid_t), sizeof(old_gid_t), 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_prctl] = SyscallDesc{5, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_rt_sigtimedwait] = SyscallDesc{4, 0, 1, {0, sizeof(siginfo_t), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_rt_sigqueueinfo] = SyscallDesc{3, 0, 1, {0, 0, sizeof(siginfo_t), 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_rt_sigsuspend] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_pread64] = SyscallDesc{4, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postReadHook};
  kSyscallDesc[__NR_pwrite64] = SyscallDesc{4, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_chown] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getcwd] = SyscallDesc{2, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postGetCwdHook};
  kSyscallDesc[__NR_capget] = SyscallDesc{2, 0, 1, {sizeof(cap_user_header_t), sizeof(cap_user_data_t), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_capset] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sigaltstack] = SyscallDesc{2, 0, 1, {0, sizeof(stack_t), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sendfile] = SyscallDesc{4, 0, 1, {0, 0, sizeof(off_t), 0, 0, 0}, NULL, NULL};
  // kSyscallDesc[__NR_streams1] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  // kSyscallDesc[__NR_streams2] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_vfork] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getrlimit] = SyscallDesc{2, 0, 1, {0, sizeof(struct rlimit), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_lchown] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getuid] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getgid] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_geteuid] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getegid] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_setreuid] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_setregid] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getgroups] = SyscallDesc{2, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postGetGroupsHook};
  kSyscallDesc[__NR_setgroups] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_fchown] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_setresuid] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getresuid] = SyscallDesc{3, 0, 1, {sizeof(uid_t), sizeof(uid_t), sizeof(uid_t), 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_setresgid] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getresgid] = SyscallDesc{3, 0, 1, {sizeof(gid_t), sizeof(gid_t), sizeof(gid_t), 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_chown] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_setuid] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_setgid] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_setfsuid] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_setfsgid] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_pivot_root] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_mincore] = SyscallDesc{3, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postMincoreHook};
  kSyscallDesc[__NR_madvise] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getdents] = SyscallDesc{3, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postGetDentsHook};
  // __NR_223 is undefined
  kSyscallDesc[__NR_gettid] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_readahead] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_setxattr] = SyscallDesc{5, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_lsetxattr] = SyscallDesc{5, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_fsetxattr] = SyscallDesc{5, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_getxattr] = SyscallDesc{4, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postGetxattrHook};
  kSyscallDesc[__NR_lgetxattr] = SyscallDesc{4, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postGetxattrHook};
  kSyscallDesc[__NR_fgetxattr] = SyscallDesc{4, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postGetxattrHook};
  kSyscallDesc[__NR_listxattr] = SyscallDesc{3, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postListXattrHook};
  kSyscallDesc[__NR_llistxattr] = SyscallDesc{3, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postListXattrHook};
  kSyscallDesc[__NR_flistxattr] = SyscallDesc{3, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postListXattrHook};
  kSyscallDesc[__NR_removexattr] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_lremovexattr] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_fremovexattr] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_tkill] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_futex] = SyscallDesc{6, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sched_setaffinity] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sched_getaffinity] = SyscallDesc{3, 0, 1, {0, 0, sizeof(cpu_set_t), 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_set_thread_area] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_get_thread_area] = SyscallDesc{1, 0, 1, {sizeof(struct user_desc), 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_io_setup] = SyscallDesc{2, 0, 1, {0, sizeof(aio_context_t), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_io_destroy] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_io_getevents] = SyscallDesc{5, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postIoGetEventsHook};
  kSyscallDesc[__NR_io_submit] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_io_cancel] = SyscallDesc{3, 0, 1, {0, 0, sizeof(struct io_event), 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_fadvise64] = SyscallDesc{4, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  // __NR_251 is undefined
  kSyscallDesc[__NR_exit_group] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_lookup_dcookie] = SyscallDesc{3, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postLookupDcookieHook};
  kSyscallDesc[__NR_epoll_create] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_epoll_ctl] = SyscallDesc{4, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_epoll_wait] = SyscallDesc{4, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postEpollWaitHook};
  kSyscallDesc[__NR_remap_file_pages] = SyscallDesc{5, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_set_tid_address] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_timer_create] = SyscallDesc{3, 0, 1, {0, 0, sizeof(timer_t), 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_timer_settime] = SyscallDesc{4, 0, 1, {0, 0, 0, sizeof(struct itimerspec), 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_timer_gettime] = SyscallDesc{2, 0, 1, {0, sizeof(struct itimerspec), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_timer_getoverrun] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_timer_delete] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_clock_settime] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_clock_gettime] = SyscallDesc{2, 0, 1, {0, sizeof(struct timespec), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_clock_getres] = SyscallDesc{2, 0, 1, {0, sizeof(struct timespec), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_clock_nanosleep] = SyscallDesc{4, 0, 1, {0, 0, 0, sizeof(struct timespec), 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_tgkill] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_utimes] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_vserver] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_mbind] = SyscallDesc{6, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_get_mempolicy] = SyscallDesc{5, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postGetMemPolicy};
  kSyscallDesc[__NR_set_mempolicy] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_mq_open] = SyscallDesc{4, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_mq_unlink] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_mq_timedsend] = SyscallDesc{5, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_mq_timedreceive] = SyscallDesc{5, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postMqTimedReceiveHook};
  kSyscallDesc[__NR_mq_notify] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_mq_getsetattr] = SyscallDesc{3, 0, 1, {0, 0, sizeof(struct mq_attr), 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_kexec_load] = SyscallDesc{4, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_waitid] = SyscallDesc{4, 0, 1, {0, 0, sizeof(siginfo_t), 0, sizeof(struct rusage), 0}, NULL, NULL};
  kSyscallDesc[__NR_add_key] = SyscallDesc{5, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_request_key] = SyscallDesc{4, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_keyctl] = SyscallDesc{5, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_ioprio_set] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_ioprio_get] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_inotify_init] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_inotify_add_watch] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_inotify_rm_watch] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_migrate_pages] = SyscallDesc{4, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_openat] = SyscallDesc{4, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_mkdirat] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_mknodat] = SyscallDesc{4, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_fchownat] = SyscallDesc{5, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_futimesat] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_unlinkat] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_renameat] = SyscallDesc{4, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_linkat] = SyscallDesc{5, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_symlinkat] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_readlinkat] = SyscallDesc{4, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postReadLinkAtHook};
  kSyscallDesc[__NR_fchmodat] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_faccessat] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_pselect6] = SyscallDesc{6, 0, 1, {0, sizeof(fd_set), sizeof(fd_set), sizeof(fd_set), 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_ppoll] = SyscallDesc{5, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postPollHook};
  kSyscallDesc[__NR_unshare] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_set_robust_list] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_get_robust_list] = SyscallDesc{3, 0, 1, {0, sizeof(struct robust_list_head*), sizeof(size_t), 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_splice] = SyscallDesc{6, 0, 1, {0, sizeof(loff_t), 0, sizeof(loff_t), 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sync_file_range] = SyscallDesc{4, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_tee] = SyscallDesc{4, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_vmsplice] = SyscallDesc{4, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_move_pages] = SyscallDesc{6, 0, 1, {0, 0, 0, 0, sizeof(int), 0}, NULL, NULL};
  kSyscallDesc[__NR_getcpu] = SyscallDesc{3, 0, 1, {sizeof(unsigned), sizeof(unsigned), sizeof(struct getcpu_cache), 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_epoll_pwait] = SyscallDesc{6, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postEpollWaitHook};
  kSyscallDesc[__NR_utimensat] = SyscallDesc{4, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_signalfd] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_timerfd_create] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_eventfd] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_fallocate] = SyscallDesc{4, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_timerfd_settime] = SyscallDesc{4, 0, 1, {0, 0, 0, sizeof(struct itimerspec), 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_timerfd_gettime] = SyscallDesc{2, 0, 1, {0, sizeof(struct itimerspec), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_signalfd4] = SyscallDesc{4, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_eventfd2] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_epoll_create1] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_dup3] = SyscallDesc{3, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_pipe2] = SyscallDesc{2, 0, 1, {sizeof(int)*2, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_inotify_init1] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_preadv] = SyscallDesc{5, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postReadvHook};
  kSyscallDesc[__NR_pwritev] = SyscallDesc{5, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_rt_tgsigqueueinfo] = SyscallDesc{4, 0, 1, {0, 0, 0, sizeof(siginfo_t), 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_rt_tgsigqueueinfo] = SyscallDesc{4, 0, 1, {0, 0, 0, sizeof(siginfo_t), 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_perf_event_open] = SyscallDesc{5, 0, 1, {sizeof(struct perf_event_attr), 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_recvmmsg] = SyscallDesc{5, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postRecvMmsgHook};
  kSyscallDesc[__NR_fanotify_init] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_fanotify_mark] = SyscallDesc{5, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_name_to_handle_at] = SyscallDesc{5, 0, 1, {0, 0, sizeof(struct file_handle), sizeof(int), 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_open_by_handle_at] = SyscallDesc{3, 0, 1, {0, sizeof(struct file_handle), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_clock_adjtime] = SyscallDesc{2, 0, 1, {0, sizeof(struct timex), 0, 0, 0, 0}, NULL, NULL};

#ifdef __i386__
  kSyscallDesc[__NR_waitpid] = SyscallDesc{3, 0, 1, {0, sizeof(int), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_break] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_umount] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_stime] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_stty] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_gtty] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_nice] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_ftime] = SyscallDesc{0, 0, 1, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_prof] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_signal] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_lock] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_mpx] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_ulimit] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_olduname] = SyscallDesc{1, 0, 1, {sizeof(struct oldold_utsname), 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_rt_sigaction] = SyscallDesc{4, 0, 1, {0, 0, sizeof(struct sigaction), 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sgetmask] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_ssetmask] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sigreturn] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sigprocmask] = SyscallDesc{3, 0, 1, {0, 0, sizeof(old_sigset_t), 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sigsuspend] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sigpending] = SyscallDesc{1, 0, 1, {sizeof(old_sigset_t), 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_rt_sigreturn] = SyscallDesc{1, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_rt_sigprocmask] = SyscallDesc{4, 0, 1, {0, 0, sizeof(sigset_t), 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_rt_sigpending] = SyscallDesc{2, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postRtSigPendingHook};
  kSyscallDesc[__NR_readdir] = SyscallDesc{3, 0, 1, {0, sizeof(struct old_linux_dirent), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_profil] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_socketcall] = SyscallDesc{2, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postSocketCallHook};
  kSyscallDesc[__NR_idle] = SyscallDesc{0, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_vm86old] = SyscallDesc{2, 0, 1, {sizeof(struct vm86_struct ), 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_ipc] = SyscallDesc{6, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postIpcHook};
  kSyscallDesc[__NR_bdflush] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR__llseek] = SyscallDesc{5, 0, 1, {0, 0, 0, sizeof(loff_t), 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_vm86] = SyscallDesc{3, 0, 1, {0, sizeof(struct vm86plus_struct ), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_mmap2] = SyscallDesc{6, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postMMapHook};
  kSyscallDesc[__NR_truncate64] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_ftruncate64] = SyscallDesc{2, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_stat64] = SyscallDesc{2, 0, 1, {0, sizeof(struct stat64), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_lstat64] = SyscallDesc{2, 0, 1, {0, sizeof(struct stat64), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_fstat64] = SyscallDesc{2, 0, 1, {0, sizeof(struct stat64), 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_fcntl64] = SyscallDesc{3, 1, 0, {0, 0, 0, 0, 0, 0}, NULL, postFcntlHook};
  kSyscallDesc[__NR_sendfile64] = SyscallDesc{4, 0, 1, {0, 0, sizeof(loff_t), 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_statfs64] = SyscallDesc{3, 0, 1, {0, 0, sizeof(struct statfs64), 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_fstatfs64] = SyscallDesc{3, 0, 1, {0, 0, sizeof(struct statfs64), 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_fadvise64_64] = SyscallDesc{4, 0, 0, {0, 0, 0, 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_fstatat64] = SyscallDesc{4, 0, 1, {0, 0, sizeof(struct stat64), 0, 0, 0}, NULL, NULL};
  kSyscallDesc[__NR_sigaction] = SyscallDesc{3, 0, 1, {0, 0, sizeof(struct sigaction), 0, 0, 0}, NULL, NULL};
#endif
}

int
SetSyscallPre(SyscallDesc *desc, void (* pre)(SyscallContext*))
{
	if (unlikely((desc == NULL) | (pre == NULL)))
		return 1;

	desc->pre = pre;
	desc->save_args = 1;
	return 0;
}

int
setSyscallPost(SyscallDesc *desc, void (* post)(SyscallContext*))
{
	if (unlikely((desc == NULL) | (post == NULL)))
		return 1;

	desc->post = post;
	desc->save_args = 1;
	return 0;
}

int
clearSyscallPre(SyscallDesc *desc)
{
	if (unlikely(desc == NULL))
		return 1;

	desc->pre = NULL;

	if (desc->post == NULL)
		desc->save_args = 0;
	return 0;
}

int
clearSyscallPost(SyscallDesc *desc)
{
	if (unlikely(desc == NULL))
		return 1;

	desc->post = NULL;
	if (desc->pre == NULL)
		desc->save_args = 0;
	return 0;
}

// __NR_(p)read(64) and __NR_readlink post syscall hook
static void
postReadHook(SyscallContext *ctx)
{
	// read()/readlink() was not successful
	if (unlikely((long)ctx->ret <= 0))
		return;

  // g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG1], (USIZE)ctx->ret);
}

// __NR_uselib post syscall hook
static void
postUseLibHook(SyscallContext *ctx) {
  LOG_INFO("unhandled uselib(2)\n");
}

// __NR_brk post syscall hook
static void
postBrkHook(SyscallContext *ctx)
{
	// brk() return value; in Linux brk returns
	// the address of the new program break, or
	// the current value in case of failure
	ADDRINT addr = ctx->ret;

  if (unlikely(addr == g_memory.brk_end()))
    return;

  LOG_DEBUG("brk(" + hexstr(g_memory.brk_start()) + "-" + hexstr(addr) + ")\n");
  g_memory.brk(addr);
}


static void
postGetGroups16Hook(SyscallContext *ctx) {
  postGetGroupsHookN(ctx, sizeof(old_gid_t));
}

static void
postGetGroupsHook(SyscallContext *ctx) {
  postGetGroupsHookN(ctx, sizeof(gid_t));
}

static void
postReadLinkAtHook(SyscallContext *ctx)
{
	if (unlikely((long)ctx->ret <= 0))
		return;
	g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG2], (size_t)ctx->ret);
}

void
postMMapHook(SyscallContext *ctx)
{
	// mmap parameters (size, protection, and flags)
	size_t	size	= ctx->arg[SYSCALL_ARG1];
	int	flags	= (int)ctx->arg[SYSCALL_ARG3];
  ADDRINT start, end;

	if (unlikely((void *)ctx->ret == MAP_FAILED))
		return;

	// MAP_SHARED has been specified
	// TODO: handle shared memory mappings
	if (unlikely((flags & MAP_SHARED) != 0))
    LOG_INFO("shared mapping via mmap(2) at " + hexstr(ctx->ret) + "\n");

  // MAP_FIXED has been specified
	// TODO: handle fixed memory mappings
	if (unlikely((flags & MAP_FIXED) != 0))
    LOG_INFO("fixed mapping via mmap(2) at " + hexstr(ctx->ret) + "\n");

	// MAP_GROWSDOWN has been specified
	if (unlikely((flags & MAP_GROWSDOWN) != 0)) {
    start = ctx->ret - size + 1;
    end = ctx->ret;
    LOG_DEBUG("growsdown mapping(" + hexstr(start) + "-" + hexstr(end) + ")\n");
	}
	else {
		start	= ctx->ret;
		end	= ctx->ret + size - 1;
    LOG_DEBUG("mapping(" + hexstr(start) + "-" + hexstr(end) + ")\n");
	}

  g_memory.mmap(start, end);
}

static void
postMUnmapHook(SyscallContext *ctx)
{
	ADDRINT 	addr	= ctx->arg[SYSCALL_ARG0];
	size_t	size	= ctx->arg[SYSCALL_ARG1];

	if (unlikely((int)ctx->ret == -1))
		return;

  LOG_DEBUG("unmap(" + hexstr(addr) + "-" + hexstr(addr + size - 1) + ")\n");

  // deallocate the space of the corresponding
  // mem segment by invoking munmap(2)

  g_memory.munmap(addr, size);
}

static void
postReadvHook(SyscallContext *ctx)
{
	struct	iovec *iov;
	size_t	iov_tot;
	size_t	tot = (size_t)ctx->ret;

	if (unlikely((long)ctx->ret <= 0))
		return;

	// iterate the iovec structures
	for (int i = 0; i < (int)ctx->arg[SYSCALL_ARG2] && tot > 0; i++) {
		// get an iovec
		iov = ((struct iovec *)ctx->arg[SYSCALL_ARG1]) + i;

		// get the length of the iovec
		iov_tot = (tot >= (size_t)iov->iov_len) ?
				(size_t)iov->iov_len : tot;

		g_memory.clearExprFromMem((size_t)iov->iov_base, iov_tot);

		// housekeeping
		tot -= iov_tot;
	}
}

static void
postEpollWaitHook(SyscallContext *ctx) {
	if (unlikely((long)ctx->ret <= 0))
		return;
	g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG1],
			sizeof(struct epoll_event) * (size_t)ctx->ret);
}

static void
postPollHook(SyscallContext *ctx)
{
	struct	pollfd *pfd;

	if (unlikely((long)ctx->ret <= 0))
		return;

	for (size_t i = 0; i < (size_t)ctx->arg[SYSCALL_ARG1]; i++) {
		// get pollfd
		pfd = ((struct pollfd *)ctx->arg[SYSCALL_ARG0]) + i;
		g_memory.clearExprFromMem((size_t)&pfd->revents, sizeof(short));
	}
}

static void
postMqTimedReceiveHook(SyscallContext *ctx) {
	if (unlikely((long)ctx->ret <= 0))
		return;

	g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG1], (size_t)ctx->ret);

	// priority argument is supplied
	if ((size_t *)ctx->arg[SYSCALL_ARG3] != NULL)
		g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG3], sizeof(size_t));
}

static void
postGetMemPolicy(SyscallContext *ctx) {
	if (unlikely((long)ctx->ret < 0))
		return;

	// flags is zero
	if ((unsigned long)ctx->arg[SYSCALL_ARG4] == 0) {
		g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG0], sizeof(int));
		g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG1],
						sizeof(unsigned long));
		return;
	}

	// MPOL_F_MEMS_ALLOWED is set on flags
	if (((unsigned long)ctx->arg[SYSCALL_ARG4] &
				MPOL_F_MEMS_ALLOWED) != 0) {
		g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG1],
						sizeof(unsigned long));
		return;
	}

	// MPOL_F_ADDR is set on flags
	if (((unsigned long)ctx->arg[SYSCALL_ARG4] & MPOL_F_ADDR) != 0 &&
		((unsigned long)ctx->arg[SYSCALL_ARG4] & MPOL_F_NODE) == 0) {
		// mode is provided
		if ((int *)ctx->arg[SYSCALL_ARG0] != NULL)
			g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG0],
							sizeof(int));

		// nodemask is provided
		if ((unsigned long *)ctx->arg[SYSCALL_ARG1] != NULL)
			g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG1],
						sizeof(unsigned long));
		return;
	}

	// MPOL_F_NODE & MPOL_F_ADDR is set on flags
	if (((unsigned long)ctx->arg[SYSCALL_ARG4] & MPOL_F_ADDR) != 0 &&
		((unsigned long)ctx->arg[SYSCALL_ARG4] & MPOL_F_NODE) != 0) {
		g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG0], sizeof(int));
		return;
	}

	// MPOL_F_NODE is set on flags
	if (((unsigned long)ctx->arg[SYSCALL_ARG4] & MPOL_F_NODE) != 0) {
		g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG0], sizeof(int));
		return;
	}
}

static void
postLookupDcookieHook(SyscallContext *ctx)
{
	if (unlikely((long)ctx->ret <= 0))
		return;
	g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG1], (size_t)ctx->ret);
}

static void
postIoGetEventsHook(SyscallContext *ctx)
{
	if (unlikely((long)ctx->ret <= 0))
		return;

	g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG3],
				sizeof(struct io_event) * (size_t)ctx->ret);

	// timespec is specified
	if ((struct timespec *)ctx->arg[SYSCALL_ARG4] != NULL)
		g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG4],
						sizeof(struct timespec));
}

static void
postListXattrHook(SyscallContext *ctx)
{
	if ((long)ctx->ret <= 0 || (void *)ctx->arg[SYSCALL_ARG1] == NULL)
		return;
	g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG1], (size_t)ctx->ret);
}

static void
postGetxattrHook(SyscallContext *ctx)
{
	if ((long)ctx->ret <= 0 || (void *)ctx->arg[SYSCALL_ARG2] == NULL)
		return;

	g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG2], (size_t)ctx->ret);
}

static void
postGetDentsHook(SyscallContext *ctx)
{
	if (unlikely((long)ctx->ret <= 0))
		return;

	g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG1], (size_t)ctx->ret);
}

static void
postMincoreHook(SyscallContext *ctx)
{
	if (unlikely((long)ctx->ret < 0))
		return;

	g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG2],
		(((size_t)ctx->arg[SYSCALL_ARG1] + kPageSize - 1) / kPageSize));
}

static void
postGetCwdHook(SyscallContext *ctx)
{
	if (unlikely((long)ctx->ret <= 0))
		return;

	g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG0], (size_t)ctx->ret);
}

#ifdef __x86_64__
__attribute__((unused))
#endif
static void
postRtSigPendingHook(SyscallContext *ctx)
{
	if (unlikely((long)ctx->ret < 0))
		return;

	g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG0], (size_t)ctx->arg[SYSCALL_ARG1]);
}

static void
postQuotatlHook(SyscallContext *ctx)
{
	size_t off;

	if (unlikely((long)ctx->ret < 0))
		return;

	switch ((int)ctx->arg[SYSCALL_ARG0]) {
		case Q_GETFMT:
			off = sizeof(__u32);
			break;
		case Q_GETINFO:
			off = sizeof(struct if_dqinfo);
			break;
		case Q_GETQUOTA:
			off = sizeof(struct if_dqblk);
			break;
		case Q_XGETQSTAT:
			off = sizeof(struct fs_quota_stat);
			break;
		case Q_XGETQUOTA:
			off = sizeof(struct fs_disk_quota);
			break;
		default:
			return;
	}

	g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG3], off);
}

static void
postModifyLdtHook(SyscallContext *ctx)
{
	if (unlikely((long)ctx->ret <= 0))
		return;

	g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG1], (size_t)ctx->ret);
}

#ifdef __x86_64__
__attribute__((unused))
#endif
static void
postIpcHook(SyscallContext *ctx)
{
	union semun *su;
	struct shmid_ds buf;
	size_t shm_addr;
	size_t size;

	// ipc() is a demultiplexer for all SYSV IPC calls
	switch ((int)ctx->arg[SYSCALL_ARG0]) {
		case MSGCTL:
			if (unlikely((long)ctx->ret < 0))
				return;

			// fix the cmd parameter
			ctx->arg[SYSCALL_ARG2] -= IPC_FIX;

			switch ((int)ctx->arg[SYSCALL_ARG2]) {
				case IPC_STAT:
				case MSG_STAT:
					g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG4],
						sizeof(struct msqid_ds));
					break;
				case IPC_INFO:
				case MSG_INFO:
					g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG4],
						sizeof(struct msginfo));
					break;
				default:
					return;
			}
			break;

    case SHMCTL:
			if (unlikely((long)ctx->ret < 0))
				return;

			// fix the cmd parameter
			ctx->arg[SYSCALL_ARG2] -= IPC_FIX;

			switch ((int)ctx->arg[SYSCALL_ARG2]) {
				case IPC_STAT:
				case SHM_STAT:
					g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG4],
						sizeof(struct shmid_ds));
					break;
				case IPC_INFO:
				case SHM_INFO:
					g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG4],
						sizeof(struct shminfo));
					break;
				default:
					return;
			}
			break;

    case SEMCTL:
			if (unlikely((long)ctx->ret < 0))
				return;

			// get the semun structure
			su = (union semun *)ctx->arg[SYSCALL_ARG4];

			// fix the cmd parameter
			ctx->arg[SYSCALL_ARG3] -= IPC_FIX;

			switch ((int)ctx->arg[SYSCALL_ARG3]) {
				case IPC_STAT:
				case SEM_STAT:
					g_memory.clearExprFromMem((size_t)su->buf,
						sizeof(struct semid_ds));
					break;
				case IPC_INFO:
				case SEM_INFO:
					g_memory.clearExprFromMem((size_t)su->buf,
						sizeof(struct seminfo));
					break;
				default:
					return;
			}
			break;

    case MSGRCV:
			if (unlikely((long)ctx->ret <= 0))
				return;

			g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG4],
					(size_t)ctx->ret + sizeof(long));
			break;

    case SHMAT:
			if (unlikely((long)ctx->ret < 0))
				return;

			// get the address of the attached memory segment
			shm_addr = *(size_t *)ctx->arg[SYSCALL_ARG3];

			// shared memory segment metadata
			if (unlikely(shmctl((int)ctx->arg[SYSCALL_ARG1],
						IPC_STAT, &buf) == -1))
        LOG_FATAL("reading shared memory metadata failed(" + decstr(errno) + ")\n");

      g_memory.mmap(shm_addr, shm_addr + buf.shm_segsz - 1);
			kSharedMemoryMap[shm_addr] = buf.shm_segsz;
			break;

    case SHMDT:
			if (unlikely((long)ctx->ret < 0))
				return;

			// shm address and size
			shm_addr	= ctx->arg[SYSCALL_ARG4];
			size		= kSharedMemoryMap[shm_addr];
      g_memory.munmap(shm_addr, size);
			kSharedMemoryMap.erase(shm_addr);
			break;

		default:
			return;
	}
}

static void
postFcntlHook(SyscallContext *ctx)
{
	if (unlikely((long)ctx->ret < 0))
		return;

	//  differentiate based on the cmd argument
	switch((int)ctx->arg[SYSCALL_ARG1]) {
		case F_GETLK:
      g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG2],
					sizeof(struct flock));
			break;

    case F_GETLK64:
      g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG2],
					sizeof(struct flock64));
			break;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
		case F_GETOWN_EX:
      g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG2],
					sizeof(struct f_owner_ex));
			break;
#endif
		default:
			break;
	}
}

#ifdef __x86_64__
__attribute__((unused))
#endif
static void
postSocketCallHook(SyscallContext *ctx)
{
	struct	msghdr *msg;
	size_t	iov_tot;
	struct	iovec *iov;
	size_t	tot;
	unsigned long	*args = (unsigned long *)ctx->arg[SYSCALL_ARG1];

	// demultiplex the socketcall
	switch ((int)ctx->arg[SYSCALL_ARG0]) {
		case SYS_ACCEPT:
		case SYS_ACCEPT4:
		case SYS_GETSOCKNAME:
		case SYS_GETPEERNAME:
			if (unlikely((long)ctx->ret < 0))
				return;

			// addr argument is provided
			if ((void *)args[SYSCALL_ARG1] != NULL) {
				g_memory.clearExprFromMem(args[SYSCALL_ARG1],
					*((int *)args[SYSCALL_ARG2]));

				g_memory.clearExprFromMem(args[SYSCALL_ARG2], sizeof(int));
			}
			break;
		case SYS_SOCKETPAIR:
			if (unlikely((long)ctx->ret < 0))
				return;

			g_memory.clearExprFromMem(args[SYSCALL_ARG3], (sizeof(int) * 2));
			break;
		case SYS_RECV:
			if (unlikely((long)ctx->ret <= 0))
				return;

      g_memory.clearExprFromMem(args[SYSCALL_ARG1], (size_t)ctx->ret);
			break;
		case SYS_RECVFROM:
			if (unlikely((long)ctx->ret <= 0))
				return;

      g_memory.clearExprFromMem(args[SYSCALL_ARG1], (size_t)ctx->ret);

			// sockaddr argument is specified
			if ((void *)args[SYSCALL_ARG4] != NULL) {
				g_memory.clearExprFromMem(args[SYSCALL_ARG4],
					*((int *)args[SYSCALL_ARG5]));

				g_memory.clearExprFromMem(args[SYSCALL_ARG5], sizeof(int));
			}
			break;
		case SYS_GETSOCKOPT:
			if (unlikely((long)ctx->ret < 0))
				return;

			g_memory.clearExprFromMem(args[SYSCALL_ARG3],
					*((int *)args[SYSCALL_ARG4]));

			g_memory.clearExprFromMem(args[SYSCALL_ARG4], sizeof(int));
			break;
		case SYS_RECVMSG:
			if (unlikely((long)ctx->ret <= 0))
				return;

			// extract the message header
			msg = (struct msghdr *)args[SYSCALL_ARG1];

			// source address specified
			if (msg->msg_name != NULL) {
				g_memory.clearExprFromMem((size_t)msg->msg_name,
					msg->msg_namelen);

				g_memory.clearExprFromMem((size_t)&msg->msg_namelen,
						sizeof(int));
			}

			// ancillary data specified
			if (msg->msg_control != NULL) {
				g_memory.clearExprFromMem((size_t)msg->msg_control,
					msg->msg_controllen);

				g_memory.clearExprFromMem((size_t)&msg->msg_controllen,
						sizeof(int));
			}

			// flags
			g_memory.clearExprFromMem((size_t)&msg->msg_flags, sizeof(int));

			// total bytes received
			tot = (size_t)ctx->ret;

			// iterate the iovec structures
			for (size_t i = 0; i < msg->msg_iovlen && tot > 0; i++) {
				// get the next I/O vector
				iov = &msg->msg_iov[i];

				// get the length of the iovec
				iov_tot = (tot > (size_t)iov->iov_len) ?
						(size_t)iov->iov_len : tot;

				g_memory.clearExprFromMem((size_t)iov->iov_base, iov_tot);

				// housekeeping
				tot -= iov_tot;
			}
			break;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
		case SYS_RECVMMSG:
			// fix the syscall context
			ctx->arg[SYSCALL_ARG0] = args[SYSCALL_ARG0];
			ctx->arg[SYSCALL_ARG1] = args[SYSCALL_ARG1];
			ctx->arg[SYSCALL_ARG2] = args[SYSCALL_ARG2];
			ctx->arg[SYSCALL_ARG3] = args[SYSCALL_ARG3];
			ctx->arg[SYSCALL_ARG4] = args[SYSCALL_ARG4];

			postRecvMmsgHook(ctx);
			break;
#endif
		default:
			return;
	}
}

static void
postSyslogHook(SyscallContext *ctx)
{
	if (unlikely((long)ctx->ret <= 0))
		return;

	// differentiate based on the type
	switch ((int)ctx->arg[SYSCALL_ARG0]) {
		case 2:
		case 3:
		case 4:
			g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG1], (size_t)ctx->ret);
			break;
		default:
			return;
	}
}

static void
postSysctlHook(SyscallContext *ctx)
{
	struct __sysctl_args *sa;

	if (unlikely((long)ctx->ret < 0))
		return;

	sa = (struct __sysctl_args *)ctx->arg[SYSCALL_ARG0];
	g_memory.clearExprFromMem((size_t)sa->newval, sa->newlen);

	// save old value is specified
	if (sa->oldval != NULL) {
		g_memory.clearExprFromMem((size_t)sa->oldval, *sa->oldlenp);
		g_memory.clearExprFromMem((size_t)sa->oldlenp, sizeof(size_t));
	}
}

static void
postMRemapHook(SyscallContext *ctx) {
  ADDRINT start = ctx->arg[SYSCALL_ARG0];
  size_t old_size = ctx->arg[SYSCALL_ARG1];
  size_t new_size = ctx->arg[SYSCALL_ARG2];

	if (unlikely((void *)ctx->ret == MAP_FAILED))
		return;

  g_memory.mremap(start, old_size, ctx->ret, new_size);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
static void
postRecvMmsgHook(SyscallContext *ctx)
{
	struct	mmsghdr *msg;
	struct	msghdr *m;
	size_t	iov_tot;

	struct	iovec *iov;
	size_t	tot;

	if (unlikely((long)ctx->ret < 0))
		return;

	// iterate the mmsghdr structures
	for (size_t i = 0; i < (size_t)ctx->ret; i++) {
		// get the next mmsghdr structure
		msg = ((struct mmsghdr *)ctx->arg[SYSCALL_ARG1]) + i;

		// extract the message header
		m = &msg->msg_hdr;

		// source address specified
		if (m->msg_name != NULL) {
			g_memory.clearExprFromMem((size_t)m->msg_name, m->msg_namelen);
			g_memory.clearExprFromMem((size_t)&m->msg_namelen, sizeof(int));
		}

		// ancillary data specified
		if (m->msg_control != NULL) {
			g_memory.clearExprFromMem((size_t)m->msg_control, m->msg_controllen);
			g_memory.clearExprFromMem((size_t)&m->msg_controllen, sizeof(int));
		}

		// flags
		g_memory.clearExprFromMem((size_t)&m->msg_flags, sizeof(int));

		// total bytes received
		tot = (size_t)msg->msg_len;
		g_memory.clearExprFromMem((size_t)&msg->msg_len, sizeof(unsigned));

		// iterate the iovec structures
		for (size_t j = 0; j < m->msg_iovlen && tot > 0; j++) {
			// get the next I/O vector
			iov = &m->msg_iov[j];

			// get the length of the iovec
			iov_tot = (tot > (size_t)iov->iov_len) ?
					(size_t)iov->iov_len : tot;

			g_memory.clearExprFromMem((size_t)iov->iov_base, iov_tot);

			// housekeeping
			tot -= iov_tot;
		}
	}

	// timespec structure specified
	if ((struct timespec *)ctx->arg[SYSCALL_ARG4] != NULL)
		g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG4], sizeof(struct timespec));
}
#endif

} // namespace qsym
