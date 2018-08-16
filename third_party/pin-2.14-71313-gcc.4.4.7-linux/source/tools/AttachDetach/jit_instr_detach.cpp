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
 * This tool is meant to be used with the mt_thread.cpp application. It tests the correctness of PIN_AddDetachFunction.
 * We expect to get exactly one detach callback regardless of the number of threads in the application or the number of
 * detach requests.
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cassert>
#include "pin.H"
#include "atomic.hpp"

using namespace std;

KNOB<string> KnobOutFile(KNOB_MODE_WRITEONCE,  "pintool",
    "o", "jit_instr_detach.out", "Specify file name for the tool's output.");

ofstream outfile;

volatile UINT32 instrumentedInstructions = 0; // the number of instructions that were instrumented.
volatile UINT32 runtimeCount = 0; // the number of executed instructions until detaching.
INT32 threadCounter = 0;
PIN_LOCK lock;
volatile bool detached = false;


VOID docount()
{
    ATOMIC::OPS::Increment<UINT32>(&runtimeCount, (UINT32)1); // the instrumented application may be multi-threaded
}

    
VOID Instruction(INS ins, VOID *v)
{
    ++instrumentedInstructions;
    if (threadCounter >= 2)
    {
        OS_THREAD_ID tid = PIN_GetTid();
        outfile << "Thread " << tid << " is requesting detach." << endl;
        PIN_Detach();
    }
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END);
}


VOID Detach(VOID *v)
{
    if (detached) // sanity check
    {
        // This should never be reached because only one detach request should be executed.
        cerr << "TOOL ERROR: jit_instr_detach is executing the Detach callback twice." << endl;
        exit(20); // use exit instead of PIN_ExitProcess because we don't know if it is available at this point.
    }
    detached = true;
    outfile << "Pin detached after " << instrumentedInstructions << " instrumented instructions." << endl;
    outfile << "Pin detached after " << runtimeCount << " executed instructions." << endl;
    outfile.close();
}


VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v)
{
    OS_THREAD_ID tid = PIN_GetTid();
    PIN_GetLock(&lock, tid);
    outfile << "Thread " << tid << " has started." << endl;
    ++threadCounter;
    PIN_ReleaseLock(&lock);
}


VOID ThreadEnd(THREADID threadid, const CONTEXT *ctxt, INT32 code, VOID *v)
{
    if (detached) // sanity check
    {
        // This should never be reached because the detach callback is called after all thread fini callbacks.
        cerr << "TOOL ERROR: jit_instr_detach - executing thread fini callback after detach." << endl;
        exit(30); // use exit instead of PIN_ExitProcess because we don't know if it is available at this point.
    }
    OS_THREAD_ID tid = PIN_GetTid();
    PIN_GetLock(&lock, tid);
    outfile << "Thread " << tid << " has ended." << endl;
    assert(threadCounter > 0);
    --threadCounter;
    PIN_ReleaseLock(&lock);
}


int main(int argc, char * argv[])
{
    PIN_Init(argc, argv);

    outfile.open(KnobOutFile.Value().c_str());
    if (!outfile.is_open() || outfile.fail())
    {
        cerr << "Failed to open output file " << KnobOutFile.Value().c_str() << "." << endl;
        PIN_ExitProcess(10);
    }

    PIN_InitLock(&lock);

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadEnd, 0);
    PIN_AddDetachFunction(Detach, 0);
    
    // Never returns
    PIN_StartProgram();
    
    return 0;
}
