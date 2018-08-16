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
 * A tool that creates internal Pin thread
 * and verifies that the thread is finished gracefully.
 */

#include "pin.H"
#include <string>
#include <iostream>

namespace WIND
{
#include <windows.h>
}

using namespace std;

/*!
 * Global variables.
 */

// UID of the internal thread. It is created in the application thread by the
// main() tool's procedure.
PIN_THREAD_UID intThreadUid; 
// Pointer to TID of internal thread. Imported from application.
unsigned int * pTid;

//==========================================================================
// Utilities
//==========================================================================

/*!
 * Print out the error message and exit the process.
 */
static void AbortProcess(const string & msg, unsigned long code)
{
    cerr << "Test aborted: " << msg << " with code " << code << endl;
    PIN_WriteErrorMessage(msg.c_str(), 1002, PIN_ERR_FATAL, 0);
}

/*!
 * Internal tool's thread. It is created in the application thread by the
 * main() tool's procedure.
 */
static VOID IntThread(VOID * arg)
{
    WIND::HMODULE exeHandle = WIND::GetModuleHandle(NULL);
    pTid = (unsigned int *)WIND::GetProcAddress(exeHandle, "tid");

    // Sets current TID in imported variable, makes it available to application.
    *pTid = WIND::GetCurrentThreadId();

    // tid value is reset in Fini.
    while (*pTid != 0)
    {
        PIN_Sleep(10);
    }

    // Finishes gracefully if application doesn't harm the thread.
    PIN_ExitThread(0);
}

//==========================================================================
// Instrumentation callbacks
//==========================================================================
/*!
 * Process exit callback (unlocked).
 */
static VOID FiniUnlocked(INT32 code, VOID *v)
{
    BOOL waitStatus;
    INT32 threadExitCode;

    if (code != 0)
    {
        AbortProcess("Application exited abnormally", code);
    }

    // Notify internal thread to finish.
    *pTid = 0;

    // First, wait for termination of the main internal thread. When this thread exits,
    // all secondary internal threads are already created and, so <uidSet> can be safely
    // accessed without lock.
    waitStatus = PIN_WaitForThreadTermination(intThreadUid, 1000, &threadExitCode);
    if (!waitStatus)
    {
        AbortProcess("PIN_WaitForThreadTermination(RootThread) failed", 0);
    }
    if (threadExitCode != 0)
    {
        AbortProcess("Tool's thread exited abnormally", threadExitCode);
    }

    cerr << "Tool's thread finished successfully." << endl;
}

/*!
 * The main procedure of the tool.
 */
int main(int argc, char *argv[])
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    PIN_AddFiniUnlockedFunction(FiniUnlocked, 0);

    // Spawn the main internal thread. When this thread starts it spawns all other internal threads.
    THREADID intThreadId = PIN_SpawnInternalThread(IntThread, NULL, 0, &intThreadUid);
    if (intThreadId == INVALID_THREADID)
    {
        AbortProcess("PIN_SpawnInternalThread(intThread) failed", 0);
    }

    // Never returns
    PIN_StartProgram();
    return 0;
}
