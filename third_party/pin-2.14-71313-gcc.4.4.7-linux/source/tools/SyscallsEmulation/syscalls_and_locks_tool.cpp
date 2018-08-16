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
/*!
 * This tool should run with the application "syscalls_and_locks_app.cpp". See full details in the application source code.
 */

#include "pin.H"
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

// This knob represents which type of lock should be acquired:
// 1 - PIN_LOCK
// 2 - PIN_RWMUTEX
// 3 - CLIENT LOCK
KNOB<UINT> KnobLockType(KNOB_MODE_WRITEONCE, "pintool", "lock_type", "0", "lock_type");
 
volatile bool isLockAcquired = false;

PIN_LOCK _lock;
 
PIN_RWMUTEX _rw_mutex;
 
VOID Replacement_AcquireAndReleaseLock()
{
    switch(KnobLockType)
    {
        case(1):
        {
            PIN_InitLock(&_lock);
            PIN_GetLock(&_lock, 1);
            break;
        }
        case(2):
        {
            PIN_RWMutexInit(&_rw_mutex);
            PIN_RWMutexWriteLock(&_rw_mutex);
            break;
        }
        case(3):
        {
            PIN_LockClient();
            break;
        }
        default:
		{
            break;
	    }
    }
	
    // Notify the application that the lock has been acquired.
    isLockAcquired = true;

    // Sleep 20 seconds in order to let t1 to call to relevant system call/function
    // while t2 holds the lock.
    sleep(20);

    switch(KnobLockType)
    {
         case(1):
         {
            PIN_ReleaseLock(&_lock);
            break;
         }
         case(2):
         {
            PIN_RWMutexUnlock(&_rw_mutex);
            break;
         }
         case(3):
         {
            PIN_UnlockClient();
            break;
         }
         default:
             break;
    }
}
 
VOID Replacement_WaitThread2AcquireLock()
{
    while(!isLockAcquired) sched_yield();
}
 
VOID ImageLoad(IMG img, void *v)
{
    if(IMG_IsMainExecutable(img))
    {
        RTN rtn = RTN_Invalid();
 
        rtn = RTN_FindByName(img, "WaitThread2AcquireLock");
        if (RTN_Valid(rtn))
        {
            RTN_ReplaceProbed(rtn, AFUNPTR(Replacement_WaitThread2AcquireLock));
        }

        rtn = RTN_FindByName(img, "WaitUntilLockAcquiredAndReleased");
        if (RTN_Valid(rtn))
        {
            RTN_ReplaceProbed(rtn, AFUNPTR(Replacement_AcquireAndReleaseLock));
        }
    }
}
/* ===================================================================== */
 
int main(int argc, CHAR *argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc,argv);
 
    IMG_AddInstrumentFunction(ImageLoad,0);
 
    PIN_StartProgramProbed();
 
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
 
