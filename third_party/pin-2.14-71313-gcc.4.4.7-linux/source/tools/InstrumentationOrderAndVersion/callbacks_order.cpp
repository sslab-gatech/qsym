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
 * This tool calls PIN_AddThreadFiniFunction, PIN_AddFiniFunction, and PIN_AddThreadStartFunction.
 * Afterward, it calls CALLBACK_SetExecutionPriority() to set the relative order between
 * all the registered callbacks
 */
#include "pin.H"
#include <fstream>

using namespace std;

static KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
                                   "o", "callbacks_order.out", "specify file name");

static ofstream * out;

static VOID ThreadStart(THREADID threadIndex, CONTEXT *ctxt, INT32 flags, VOID *v)
{
    *out << dec << "ThreadStart " << ADDRINT(v) << endl;
}

static VOID ThreadFini(THREADID tid, CONTEXT const * c, INT32 code, VOID *v)
{
    *out << dec << "ThreadFini " << ADDRINT(v) << endl;
}

static VOID FiniB(INT32 code, VOID *v)
{
    *out << dec << "FiniB " << ADDRINT(v) << endl;
}

static VOID FiniA(INT32 code, VOID *v)
{
    *out << dec << "FiniA " << ADDRINT(v) << endl;
    out->close();
    delete out;
}

int main(INT32 argc, CHAR **argv)
{
    PIN_Init(argc, argv);

    out = new ofstream(KnobOutputFile.Value().c_str(), std::ofstream::out);

    PIN_CALLBACK cbThreadFini0 = PIN_AddThreadFiniFunction(ThreadFini, (VOID*)0);
    PIN_CALLBACK cbThreadFini4 = PIN_AddThreadFiniFunction(ThreadFini, (VOID*)4);
    PIN_CALLBACK cbThreadFini3 = PIN_AddThreadFiniFunction(ThreadFini, (VOID*)3);

    PIN_CALLBACK cbAppFiniA = PIN_AddFiniFunction(FiniA, 0);
    PIN_CALLBACK cbAppFiniB = PIN_AddFiniFunction(FiniB, 0);

    PIN_CALLBACK cbThreadStart2 = PIN_AddThreadStartFunction(ThreadStart, (VOID*)2);
    PIN_CALLBACK cbThreadStart0 = PIN_AddThreadStartFunction(ThreadStart, (VOID*)0);
    PIN_CALLBACK cbThreadStart1 = PIN_AddThreadStartFunction(ThreadStart, (VOID*)1);


    CALLBACK_SetExecutionPriority(cbThreadFini0, CALL_ORDER_DEFAULT+0);
    CALLBACK_SetExecutionPriority(cbThreadFini4, CALL_ORDER_DEFAULT+4);
    CALLBACK_SetExecutionPriority(cbThreadFini3, CALL_ORDER_DEFAULT+3);

    CALLBACK_SetExecutionPriority(cbThreadStart2, CALL_ORDER_DEFAULT+2);
    CALLBACK_SetExecutionPriority(cbThreadStart0, CALL_ORDER_DEFAULT+0);
    CALLBACK_SetExecutionPriority(cbThreadStart1, CALL_ORDER_DEFAULT+1);

    CALLBACK_SetExecutionPriority(cbAppFiniA, CALL_ORDER_DEFAULT+10);
    CALLBACK_SetExecutionPriority(cbAppFiniB, CALL_ORDER_DEFAULT+1);

    // Never returns
    PIN_StartProgram();

    return 0;
}
