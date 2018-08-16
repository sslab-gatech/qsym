/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2015 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
/*
 * This test verifies that Pin can change system call numbers and arguments.
 * Use in conjuction with the "change_syscall.c" application.
 */

#include <sys/syscall.h>
#include <signal.h>
#include <string.h>
#include "pin.H"

#if defined(TARGET_ANDROID) && !defined(TARGET_NDK64)
#define SYS_sigaction __NR_sigaction
#define SYS_rt_sigaction __NR_rt_sigaction
#define SYS_exit __NR_exit
#define SYS_open __NR_open
#define SYS__syscall __NR__syscall
#define SYS_getpid __NR_getpid
#endif

BOOL IsSigaction(ADDRINT sysnum)
{
#ifdef SYS_sigaction
    if (sysnum == SYS_sigaction)
        return TRUE;
#endif
#ifdef SYS_rt_sigaction
    if (sysnum == SYS_rt_sigaction)
        return TRUE;
#endif
    return FALSE;
}


VOID OnSyscallEntry(THREADID threadIndex, CONTEXT *ctxt, SYSCALL_STANDARD std, VOID *v) 
{
    ADDRINT sysnum = PIN_GetSyscallNumber(ctxt, std);
    ADDRINT arg0 = PIN_GetSyscallArgument(ctxt, std, 0);

#if defined(TARGET_MAC)
# if defined(TARGET_IA32E)
    // Extract the syscall number without the UNIX mask
    sysnum = sysnum & (~(0xFF << 24));
# else
    // Extract the syscall number without the int80 mask
    sysnum = sysnum & 0xFFFF;
# endif
#endif

    char* filetoopen = NULL;
#ifdef SYS_openat
    if (sysnum == SYS_openat)
    {
        // open() is implemented using openat() in newer Bionic versions. The file is held in the second argument.
        ADDRINT arg1 = (PIN_GetSyscallArgument(ctxt, std, 1));
        filetoopen = reinterpret_cast<char *>(arg1);
    }
#endif
    if (sysnum == SYS_open)
    {
        filetoopen = reinterpret_cast<char *>(arg0);
    }
    if (filetoopen != NULL && strncmp(filetoopen, "does-not-exist1", sizeof("does-not-exist1")-1) == 0)
    {
        PIN_SetSyscallNumber(ctxt, std, SYS_getpid);
    }

    if (IsSigaction(sysnum) && (arg0 == SIGUSR1))
    {
        PIN_SetSyscallNumber(ctxt, std, SYS_getpid);
    }

    if (filetoopen && strncmp(filetoopen, "does-not-exist2", sizeof("does-not-exist2")-1) == 0)
    {
        PIN_SetSyscallNumber(ctxt, std, SYS_exit);
        PIN_SetSyscallArgument(ctxt, std, 0, 0);
    }
}


int main(int argc, char * argv[])
{
    PIN_Init(argc, argv);

    PIN_AddSyscallEntryFunction(OnSyscallEntry, 0);

    PIN_StartProgram();
    return 0;
}
