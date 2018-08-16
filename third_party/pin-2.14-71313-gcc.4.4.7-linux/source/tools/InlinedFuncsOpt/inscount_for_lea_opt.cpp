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

// The running count of instructions is kept here
// make it static to help the compiler optimize docount
static UINT64 icount1 = 0;
static UINT64 icount2 = 0;
static UINT64 icount3 = 0;
static UINT64 icount4 = 0;
static UINT64 icount5 = 0;
static UINT64 thenCount = 0;

VOID docount() { icount1++; }
extern "C" VOID UpdateIcountByAdd(UINT64 *icount_ptr);
extern "C" VOID UpdateIcountByInc(UINT64 *icount_ptr);
extern "C" VOID UpdateIcountByDecInc(UINT64 *icount_ptr);
extern "C" VOID UpdateIcountBySub(UINT64 *icount_ptr);
extern "C" VOID IfFuncWithAddThatCannotBeChangedToLea(UINT64 *icount_ptr);

VOID ThenFuncThatShouldNeverBeCalled()
{
    thenCount++;
}
    
VOID Instruction(INS ins, VOID *v)
{
    
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END);
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)UpdateIcountByAdd, IARG_PTR, &icount2, IARG_END);
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)UpdateIcountByInc, IARG_PTR, &icount3, IARG_END);
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)UpdateIcountByDecInc, IARG_PTR, &icount4, IARG_END);
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)UpdateIcountBySub, IARG_PTR, &icount5, IARG_END);
    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfFuncWithAddThatCannotBeChangedToLea, IARG_END);
    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)ThenFuncThatShouldNeverBeCalled, IARG_END);
}

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "inscount_for_lea_opt.out", "specify output file name");

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    if (icount1 != icount2 || icount1 != icount3 || icount1 != icount4 || icount1 != icount5
        || thenCount!=0)
    {
        // Write to a file since cout and cerr maybe closed by the application
        OutFile.open(KnobOutputFile.Value().c_str());
        OutFile.setf(ios::showbase);
        if (thenCount!=0)
        {
            OutFile << "****ERROR thenCount was expected to be 0  not: " << thenCount << endl;
        }
        else
        {
            OutFile << "Count1 " << icount1 << endl;
            OutFile << "Count2 " << icount2 << endl;
            OutFile << "Count3 " << icount3 << endl;
            OutFile << "Count4 " << icount3 << endl;
            OutFile << "Count5 " << icount3 << endl;
            OutFile << "***ERROR - mismatch in icounts " << endl;
        }
        OutFile.close();
        exit (1);
    }
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool counts the number of dynamic instructions executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
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

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
