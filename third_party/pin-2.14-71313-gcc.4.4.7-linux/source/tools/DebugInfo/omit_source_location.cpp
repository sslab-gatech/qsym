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

/* =====================================================================
 * This PIN tools instruments instructions in an invalid way such
 * that it will trigger an assertion.
 * The assertion that will be triggered is defined by the tool's command
 * line argument "-e" and can assume 3 possible values:
 * 0 - No assertion triggered
 * 1 - Triggers the assertion: IARG_MEMORY*_EA is only valid at IPOINT_BEFORE
 * 2 - Triggers the assertion: IARG_MEMORYOP_EA/IARG_MEMORYOP_MASKED_ON invalid memory operand index
   ===================================================================== */

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include "pin.H"

KNOB<int> KnobAssertionNumber(KNOB_MODE_WRITEONCE, "pintool",
    "e", "0", "specify assertion number to trigger (0 - 2)");
ofstream OutFile;

static UINT64 icount = 0;

VOID docount() { icount++; }

using namespace std;

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{
    switch (KnobAssertionNumber.Value())
    {
        case 0:
            // No assertion triggered
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END);
            break;
        case 1:
            // Triggers the assertion: IARG_MEMORY*_EA is only valid at IPOINT_BEFORE
            if (INS_IsMemoryRead(ins))
            {
                INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)docount, IARG_MEMORYREAD_EA, IARG_END);
            }
            break;
        case 2:
            // Triggers the assertion: IARG_MEMORYOP_EA/IARG_MEMORYOP_MASKED_ON invalid memory operand index
            // by trying to pass an out of range operand number to the analysis routine
            if (INS_IsMemoryRead(ins))
            {
                INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)docount, IARG_MEMORYOP_EA, INS_MemoryOperandCount(ins) + 1, IARG_END);
            }
            break;
    }
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool triggers an assertion in various ways" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

int main(int argc, char **argv)
{
    if (PIN_Init(argc, argv))
    {
        Usage();
        return EXIT_FAILURE;
    }
    if (KnobAssertionNumber.Value() < 0 || KnobAssertionNumber.Value() > 2)
    {
        cerr << "Assertion number must be between 0 to 2" << endl;
        return EXIT_FAILURE;
    }

    INS_AddInstrumentFunction(Instruction, NULL);

    PIN_StartProgram();
    return EXIT_FAILURE;
}
