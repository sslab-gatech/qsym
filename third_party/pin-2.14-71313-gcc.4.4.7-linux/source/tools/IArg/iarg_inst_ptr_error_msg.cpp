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
 * Iarg_inst_ptr_error_msg.cpp
 *
 *  Created on: Jul 20, 2014
 *      Author: keshchar
 */

#include "pin.H"
#include <iostream>
#include <fstream>

using namespace std;

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,        "pintool",
    "o", "iarg_explicit_memory_ea.out", "specify output file name");

KNOB<UINT32> KnobTestedRegAPI(KNOB_MODE_WRITEONCE,        "pintool",
    "reg_api", "1", "specify 1 IARG_REG_REFERENCE or 2 for IARG_RETURN_REG or 3 for const reference check");

static ofstream* out = NULL;
UINT32 test_reg_api = 1;


/* =====================================================================
 * Called upon bad command line argument
 * ===================================================================== */
INT32 Usage()
{
    cerr <<
        "This pin tool instruments Trace with IARG_INST_PTR "
            <<" to make sure proper error message is printed" << endl;

    cerr << KNOB_BASE::StringKnobSummary();

    cerr << endl;

    return -1;
}


/* =====================================================================
 * The analysis routine that is instrumented before any memory operand instruction
 * ===================================================================== */
VOID foo(ADDRINT* pc)
{
    *out << "This should never be read since an error should occur earlier" << endl;
}


VOID const_ref(ADDRINT* pc, ADDRINT pc2)
{
    if (!(*pc == pc2)) {
       *out << "ERROR value in CONST_REFERENCE is not like IARG_INST_PTR:" << endl;
       *out << hex << "const ref = " << pc << " value =" << *pc << " IARG_INST_PTR = " << pc2 << endl;
       ASSERTX(0);
    }
    *out << "Great place to work!" << endl;
}

/* =====================================================================
 * Try to instrument Instruction Pointer Register per trace.
 * ===================================================================== */
VOID Trace(TRACE trace, VOID *v)
{
    BBL bbl = TRACE_BblHead(trace);
    if (BBL_Valid(bbl)) {
        INS ins = BBL_InsHead(bbl);
        if (INS_Valid(ins)) {
            switch (test_reg_api) {
                case 1:
                    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(foo), IARG_REG_REFERENCE, REG_INST_PTR, IARG_END );
                    break;
                case 2:
                    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(foo), IARG_RETURN_REGS, REG_INST_PTR, IARG_END );
                    break;
                case 3:
                    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(const_ref), IARG_REG_CONST_REFERENCE, REG_INST_PTR,
                            IARG_INST_PTR, IARG_END);
                    break;
                default:
                    ASSERTQ("Unknown test type (reg_api), see usage for available values");

            }

            if (test_reg_api == 1) {

            } else {

            }
        }
    }
}


/* =====================================================================
 * Entry point for the tool
 * ===================================================================== */
int main(int argc, CHAR *argv[])
{
    PIN_InitSymbols();
    if ((!(test_reg_api == 1) && !(test_reg_api == 2) && !(test_reg_api == 3)) ||
         PIN_Init(argc, argv))
    {
        return Usage();
    }

    out = new std::ofstream(KnobOutputFile.Value().c_str());
    test_reg_api = KnobTestedRegAPI.Value();
    TRACE_AddInstrumentFunction(Trace, 0);

    // Never returns
    PIN_StartProgram();
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
