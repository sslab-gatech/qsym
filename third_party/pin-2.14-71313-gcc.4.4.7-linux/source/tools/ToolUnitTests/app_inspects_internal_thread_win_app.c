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
/*! @file
 * This test app examines behavior of OS thread management APIs
 * applied by application to a Pin internal thread.
 * Checked APIs are SuspendThread, GetThreadContext, SetThreadContext, ResumeThread
 * and TerminateThread.
 */

#include <windows.h>

// Exported. Tid of a Pin internal thread is set by Pin tool.
__declspec(dllexport) const unsigned int tid;

int main()
{
    int i;
    for (i = 0; i < 10; i++)
    {
        Sleep(500);
        // tid value is set by Pin tool.
        if (tid != 0)
        {
            CONTEXT context;
            DWORD count;
            BOOL res;
            HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, tid);

            count = SuspendThread(hThread);
            if (count == (DWORD)-1) return 2;

            context.ContextFlags = CONTEXT_FULL;
            if (!GetThreadContext(hThread, &context)) return 3;

            if (!SetThreadContext(hThread, &context)) return 4;

            count = ResumeThread(hThread);
            if (count == (DWORD)-1) return 5;

            // Tries to terminate thread with exit code 7.
            // The call should not succeed.
            if (TerminateThread(hThread, 7)) return 6;

            // Success.
            return 0;
        }
    }
    // tid of Pin internal thread was not set.
    return 1;
}
