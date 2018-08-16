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
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include "pin.H"

using namespace std;

static const int WAIT_FOR_THREAD1_SECONDS = 10;
static const int PIN_DETACH_TIMEOUT_SECONDS = 60;
static volatile int counter = 0;
static volatile int reproduceBugOnThisIteration = false;
static PIN_LOCK waitToMainThreadLock;
static PIN_LOCK waitToSecondThreadLock;


void AlarmRaised(int signum)
{
    if (SIGALRM == signum)
    {
        printf("*** ERROR: Timeout waiting for PIN to detach\n");
        exit(2);
    }
}

VOID SecondThreadIterationCheckpoint1_Before()
{
    if (++counter >= 10)
    {
        reproduceBugOnThisIteration = true;
        PIN_GetLock(&waitToSecondThreadLock, 0);
        PIN_GetLock(&waitToMainThreadLock, 0);
        printf("Thread 2: String detach bug reproducing sequence\n");
    }
}

VOID SecondThreadIterationCheckpoint2_Before()
{
    if (reproduceBugOnThisIteration)
    {
        printf("Thread 2: Application mutex released - waiting to sync with thread 1\n");
        PIN_GetLock(&waitToMainThreadLock, 0);
    }
}

VOID SecondThreadIterationCheckpoint3_Before()
{
    if (reproduceBugOnThisIteration)
    {
        GetVmLock();
        printf("Thread 2: Application mutex and VM lock acquired. Waiting %d seconds for the main thread\n", WAIT_FOR_THREAD1_SECONDS);
        PIN_ReleaseLock(&waitToSecondThreadLock);
        sleep(WAIT_FOR_THREAD1_SECONDS);
        printf("Thread 2: Finished waiting. Now telling PIN to detach from process - giving it %d seconds to do so\n", PIN_DETACH_TIMEOUT_SECONDS);

        signal(SIGALRM, AlarmRaised);
        alarm(PIN_DETACH_TIMEOUT_SECONDS);

        PIN_Detach();
        PIN_RemoveInstrumentation();
        ReleaseVmLock();
    }
}

VOID MainThreadIterationCheckpoint_Before()
{
    if (reproduceBugOnThisIteration)
    {
        printf("Thread 1: About to acquire application mutex\n");
        PIN_ReleaseLock(&waitToMainThreadLock);
        PIN_GetLock(&waitToSecondThreadLock, 0);
        printf("Thread 1: Going to enter a futex syscall...\n");
    }
}

VOID InstrumentRtnBefore(IMG img, const string& name, VOID(*newFn)())
{
    RTN rtn = RTN_FindByName(img, name.c_str());
    ASSERT(RTN_Valid(rtn), "Failed to find RTN " + name);
    RTN_Open(rtn);
    INS_InsertCall(RTN_InsHead(rtn), IPOINT_BEFORE, (AFUNPTR)newFn, IARG_END);
    RTN_Close(rtn);
}

VOID Image(IMG img, VOID *v)
{
    if (IMG_IsMainExecutable(img))
    {
        InstrumentRtnBefore(img, "SecondThreadIterationCheckpoint1", SecondThreadIterationCheckpoint1_Before);
        InstrumentRtnBefore(img, "SecondThreadIterationCheckpoint2", SecondThreadIterationCheckpoint2_Before);
        InstrumentRtnBefore(img, "SecondThreadIterationCheckpoint3", SecondThreadIterationCheckpoint3_Before);
        InstrumentRtnBefore(img, "MainThreadIterationCheckpoint", MainThreadIterationCheckpoint_Before);
    }
}


int main(int argc, char * argv[])
{
    PIN_InitSymbols();

    PIN_Init(argc,argv);

    PIN_InitLock(&waitToMainThreadLock);
    PIN_InitLock(&waitToSecondThreadLock);

    IMG_AddInstrumentFunction(Image, 0);

    // Never returns
    PIN_StartProgram();
    
    return 0;
}
