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
#include <iostream>
#include <fstream>
#include "pin.H"

ofstream OutFile;



extern "C" VOID IfProc1();
extern "C" VOID IfProc2();
extern "C" VOID IfProc3();
extern "C" VOID IfProc4();
extern "C" VOID IfProc5();
extern "C" VOID IfProc6();
extern "C" unsigned int globVal[];
unsigned int globVal[2];

int numTimesThenProc1Called = 0;
void ThenProc1()
{
    numTimesThenProc1Called++;
}

int ifProc2Param = 0;

int numTimesThenProc2Called = 0;
void ThenProc2()
{
    numTimesThenProc2Called++;
}
    
BOOL instrumented = FALSE;
VOID Instruction(INS ins, VOID *v)
{
    if (!instrumented)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc1, IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc2, IARG_END);
        //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc3, IARG_END);
        //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc4, IARG_END);
        //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc5, IARG_END);
        //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc6, IARG_END);
        instrumented = TRUE;
    }

    //INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfProc2, IARG_PTR, &ifProc2Param, IARG_END);
    //INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(ThenProc2), IARG_END);
}

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "inscount.out", "specify output file name");

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    // Write to a file since cout and cerr maybe closed by the application
    OutFile.setf(ios::showbase);
    OutFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
/*   argc, argv are the entire command line: pin -t <toolname> -- ...    */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    OutFile.open(KnobOutputFile.Value().c_str());

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
