#include "syscall_hook.h"
#include "memory.h"

#include <unistd.h>
#include <cstdio>
#include <set>
#include <sys/socket.h>
#include <linux/net.h>

namespace qsym {

set<int>     kFdSet;
std::string  kInput;

extern SyscallDesc  kSyscallDesc[kSyscallMax];
extern Memory g_memory;

namespace {
void postDupHook(SyscallContext *ctx) {
  if (unlikely((long)ctx->ret < 0))
    return;

  // if the old descriptor argument is
  // interesting, the returned handle is
  // also interesting
  if (likely(kFdSet.find((int)ctx->arg[SYSCALL_ARG0]) != kFdSet.end()))
    kFdSet.insert((int)ctx->ret);
}

void
postReadHook(SyscallContext *ctx) {
  if (unlikely((long)ctx->ret <= 0))
    return;

  // taint-source
  if (kFdSet.find(ctx->arg[SYSCALL_ARG0]) != kFdSet.end())
    g_memory.makeExpr(ctx->arg[SYSCALL_ARG1], ctx->ret);
  else
    g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG1], ctx->ret);
}

void
postPreadHook(SyscallContext *ctx) {
  if (unlikely((long)ctx->ret <= 0))
    return;

  // taint-source
  if (kFdSet.find(ctx->arg[SYSCALL_ARG0]) != kFdSet.end()) {
    ADDRINT old_pos = g_memory.tell();
    g_memory.lseek(ctx->arg[SYSCALL_ARG3]);
    g_memory.makeExpr(ctx->arg[SYSCALL_ARG1], ctx->ret);
    g_memory.lseek(old_pos);
  }
  else
    g_memory.clearExprFromMem(ctx->arg[SYSCALL_ARG1], ctx->ret);
}


#ifdef __i386__
void
postSocketCallHook(SyscallContext *ctx)
{
  // message header; recvmsg(2)
  struct msghdr *msg;

  // iov bytes copied; recvmsg(2)
  size_t iov_tot;

  // iterators
  struct iovec *iov;
  set<int>::iterator it;

  // total bytes received
  size_t tot;

  //socket call arguments
  unsigned long *args = (unsigned long *)ctx->arg[SYSCALL_ARG1];

  // demultiplex the socketcall
  switch ((int)ctx->arg[SYSCALL_ARG0]) {
    case SYS_SOCKET:
      if (unlikely((long)ctx->ret < 0))
        return;

      // PF_INET and PF_INET6 descriptors are
      // considered interesting
      if (likely(args[SYSCALL_ARG0] == PF_INET ||
            args[SYSCALL_ARG0] == PF_INET6))
        // add the descriptor to the monitored set
        kFdSet.insert((int)ctx->ret);
      break;
    case SYS_ACCEPT:
    case SYS_ACCEPT4:
      if (unlikely((long)ctx->ret < 0))
        return;
      // if the socket argument is interesting,
      // the returned handle of accept(2) is also
      // interesting

      if (likely(kFdSet.find(args[SYSCALL_ARG0]) !=
            kFdSet.end()))
        // add the descriptor to the monitored set
        kFdSet.insert((int)ctx->ret);
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

      // taint-source
      if (kFdSet.find((int)args[SYSCALL_ARG0]) != kFdSet.end())
        g_memory.makeExpr(args[SYSCALL_ARG1],
            (size_t)ctx->ret);
      else
        g_memory.clearExprFromMem(args[SYSCALL_ARG1],
            (size_t)ctx->ret);
      break;
    case SYS_RECVFROM:
      if (unlikely((long)ctx->ret <= 0))
        return;

      // taint-source
      if (kFdSet.find((int)args[SYSCALL_ARG0]) != kFdSet.end())
        g_memory.makeExpr(args[SYSCALL_ARG1],
            (size_t)ctx->ret);
      else
        g_memory.clearExprFromMem(args[SYSCALL_ARG1],
            (size_t)ctx->ret);

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

      // get the descriptor
      it = kFdSet.find((int)ctx->arg[SYSCALL_ARG0]);

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
        if (it != kFdSet.end())
          g_memory.makeExpr((size_t)msg->msg_control,
              msg->msg_controllen);

        else
          g_memory.clearExprFromMem((size_t)msg->msg_control,
              msg->msg_controllen);

        // clear the tag bits
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

        if (it != kFdSet.end())
          g_memory.makeExpr((size_t)iov->iov_base,
              iov_tot);
        else
          g_memory.clearExprFromMem((size_t)iov->iov_base,
              iov_tot);

        // housekeeping
        tot -= iov_tot;
      }
      break;
#if LINUX_KERNEL >= 2633
    case SYS_RECVMMSG:
#endif
    default:
      /* nothing to do */
      return;
  }
}

void postLLSeekHook(SyscallContext *ctx) {
  int fd = ctx->arg[SYSCALL_ARG0];
  loff_t* off = (loff_t*)ctx->arg[SYSCALL_ARG3];
  if (kFdSet.find(fd) != kFdSet.end())
    g_memory.lseek(*off);
  g_memory.clearExprFromMem((ADDRINT)off, sizeof(loff_t));
}
#endif

void
postCloseHook(SyscallContext *ctx) {
  set<int>::iterator it;

  if (unlikely((long)ctx->ret < 0))
    return;


  // if the descriptor (argument) is
  // interesting, remove it from the
  // monitored set
  it = kFdSet.find((int)ctx->arg[SYSCALL_ARG0]);
  if (likely(it != kFdSet.end())) {
    // TODO: handle for multiple file descriptors
    kFdSet.erase(it);
    g_memory.lseek(0);
  }
}

void
postOpenHook(SyscallContext *ctx) {
  if (unlikely((long)ctx->ret < 0))
    return;

  // ignore dynamic shared libraries
  if (strstr((char *)ctx->arg[SYSCALL_ARG0], kInput.c_str()) != NULL)
    kFdSet.insert((int)ctx->ret);
}

void
postLSeekHook(SyscallContext *ctx) {
  off_t off = ctx->ret;
  int fd = ctx->arg[SYSCALL_ARG0];
  if (kFdSet.find(fd) != kFdSet.end())
    g_memory.lseek(off);
}

void
postReadvHook(SyscallContext *ctx) {
  struct iovec *iov;
  set<int>::iterator it;
  size_t iov_tot;
  size_t tot = (size_t)ctx->ret;

  if (unlikely((long)ctx->ret <= 0))
    return;

  // get the descriptor
  it = kFdSet.find((int)ctx->arg[SYSCALL_ARG0]);

  // iterate the iovec structures
  for (int i = 0; i < (int)ctx->arg[SYSCALL_ARG2] && tot > 0; i++) {
    // get an iovec
    iov = ((struct iovec *)ctx->arg[SYSCALL_ARG1]) + i;

    // get the length of the iovec
    iov_tot = (tot >= (size_t)iov->iov_len) ?
      (size_t)iov->iov_len : tot;

    // taint interesting data and zero everything else
    if (it != kFdSet.end())
      g_memory.makeExpr((ADDRINT)iov->iov_base, iov_tot);
    else
      // clear the tag markings
      g_memory.clearExprFromMem((ADDRINT)iov->iov_base, iov_tot);

    // housekeeping
    tot -= iov_tot;
  }
}

void
postMMapHookForFile(SyscallContext *ctx)
{
  postMMapHook(ctx);

  if (unlikely((void *)ctx->ret == MAP_FAILED))
  {
    return;
  }

  // taint-source
  if (kFdSet.find(ctx->arg[SYSCALL_ARG4]) != kFdSet.end())
  {
    size_t length = (size_t)ctx->arg[SYSCALL_ARG1];
    off_t off = 0;
    // off_t off = (off_t)ctx->arg[SYSCALL_ARG5];
    // XXX: offset value is weird
    ADDRINT old_pos = g_memory.tell();
    g_memory.lseek(off);
    g_memory.makeExpr(ctx->ret, length);
    g_memory.lseek(old_pos);
  }
}

void setOpenHook() {
  // open(2), creat(2)
  (void)setSyscallPost(&kSyscallDesc[__NR_open],
      postOpenHook);
  (void)setSyscallPost(&kSyscallDesc[__NR_creat],
      postOpenHook);
  (void)setSyscallPost(&kSyscallDesc[__NR_lseek],
      postLSeekHook);
#ifdef __x86_64__
#else
  (void)setSyscallPost(&kSyscallDesc[__NR__llseek],
      postLLSeekHook);
#endif
}

void setReadHook() {
  // read(2)
  (void)setSyscallPost(&kSyscallDesc[__NR_read], postReadHook);

  // readv(2)
  (void)setSyscallPost(&kSyscallDesc[__NR_readv], postReadvHook);

  // pread
  (void)setSyscallPost(&kSyscallDesc[__NR_pread64], postPreadHook);
}

void setDupHook() {
  // dup(2), dup2(2)
  (void)setSyscallPost(&kSyscallDesc[__NR_dup], postDupHook);
  (void)setSyscallPost(&kSyscallDesc[__NR_dup2], postDupHook);
  (void)setSyscallPost(&kSyscallDesc[__NR_dup3], postDupHook);
}

void setSocketCallHook() {
#ifdef __x86_64__
  return;
#else
  (void)setSyscallPost(&kSyscallDesc[__NR_socketcall],
      postSocketCallHook);
#endif
}

void setCloseHook() {
  // close(2)
  (void)setSyscallPost(&kSyscallDesc[__NR_close], postCloseHook);
}

void setMMapHookForFile()
{
    (void)setSyscallPost(&kSyscallDesc[__NR_mmap],
        postMMapHookForFile);
#ifdef __x86_64__
#else
    (void)setSyscallPost(&kSyscallDesc[__NR_mmap2],
        postMMapHookForFile);
#endif
}

} // anonymous namespace

void hookSyscalls(bool hook_stdin, bool hook_fs, bool hook_net,
                  const std::string& input) {
  initializeSyscallDesc();

  // Save input to global variable for hook
  kInput = input;

  // Add stdin to the interesting descriptors set
  if (hook_stdin != 0)
    kFdSet.insert(STDIN_FILENO);

  if (hook_net)
    setSocketCallHook();

  if (hook_fs) {
    setMMapHookForFile();
    setOpenHook();
  }

  setReadHook();
  setCloseHook();
  setDupHook();
}

} // namespace qsym
