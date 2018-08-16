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
/**
 * This tool tests the breakpoint handling APIs.
 *
 * The test takes control over a breakpoint at the ToolControlled function.
 *
 * At first the simulates the breakpoint using PIN_ApplicationBreakpoint, 
 * then receives the breakpoint delete notifaction (which requires the tool
 * to stop causing breakpoints), then the breakpoint is re-inserted. After
 * that, it returns control back to PinADX.
 */
#include <fstream>
#include <iostream>
#include <cstdlib>
#include "pin.H"


KNOB<BOOL> KnobWaitForDebugger(KNOB_MODE_WRITEONCE, "pintool",
    "wait_for_debugger", "0", "Wait for debugger to connect");


ADDRINT _toolControlledFuncAddr = 0;
BOOL _isJobDone = FALSE;
BOOL _toolShouldStop = FALSE;

static BOOL BreakpointHandler(ADDRINT addr, UINT size, BOOL added, VOID *data);
static VOID InstrumentImg(IMG img, VOID *data);
static ADDRINT IsAtBreakpoint(ADDRINT);
static VOID ControlledBreakpoint(CONTEXT *, THREADID);
static VOID ReActivateControlledBreakpoint();


int main(int argc, char * argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    PIN_AddBreakpointHandler(BreakpointHandler, 0);
    IMG_AddInstrumentFunction(InstrumentImg, 0);

    PIN_StartProgram();
    return 0;
}

static BOOL BreakpointHandler(ADDRINT addr, UINT size, BOOL insert, VOID *data)
{
    if (_isJobDone)
        return FALSE;

    // Only interested in my controlled function
    //
    if (RTN_FindNameByAddress(addr) != "ToolControlled")
        return FALSE;

    _toolControlledFuncAddr = addr;

    // If it is a new breakpoint, we should stop.
    //
    _toolShouldStop = insert;

    return TRUE;
}

static VOID InstrumentImg(IMG img, VOID *data)
{
    if (!IMG_IsMainExecutable(img))
        return;

    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            if (RTN_Name(rtn) == "ToolControlled")
            {
                _toolControlledFuncAddr = RTN_Address(rtn);
                RTN_Open(rtn);
                for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
                {
                    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IsAtBreakpoint, IARG_INST_PTR, IARG_END);
                    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)ControlledBreakpoint, 
                            IARG_CONST_CONTEXT, IARG_THREAD_ID, IARG_END);
                }
                RTN_Close(rtn);
            }
            if (RTN_Name(rtn) == "ReActivateToolControlled")
            {
                RTN_Open(rtn);
                RTN_InsertCall(rtn, IPOINT_AFTER, AFUNPTR(ReActivateControlledBreakpoint), IARG_END);
                RTN_Close(rtn);
            }
        }
    }
}

static ADDRINT IsAtBreakpoint(ADDRINT pc)
{
    return (pc == _toolControlledFuncAddr);
}

static VOID ControlledBreakpoint(CONTEXT *ctxt, THREADID tid)
{
    if (_toolShouldStop)
    {
        _toolShouldStop = FALSE;
        PIN_ApplicationBreakpoint(ctxt, tid, KnobWaitForDebugger.Value(), "Stopped by the tool.");
    }
}

static VOID ReActivateControlledBreakpoint()
{
    _isJobDone = TRUE;
    _toolShouldStop = FALSE;
    PIN_ResetBreakpointAt(_toolControlledFuncAddr);
}

