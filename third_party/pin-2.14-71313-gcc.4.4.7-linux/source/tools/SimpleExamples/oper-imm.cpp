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
#include <cassert>
#include <iostream>
#include <iomanip>
#include <fstream>
#include "pin.H"
extern "C" {
#include "xed-interface.h"
}

/* ================================================================== */
// Global variables 
/* ================================================================== */

std::ostream* outFile = 0;

/* ===================================================================== */
// Command line switches
/* ===================================================================== */

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,  "pintool",
    "o", "", "Specify file name for the tool's output. If no filename is specified, the output will be directed to stdout.");


/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool prints information on immediate operands." << endl;
    cerr << KNOB_BASE::StringKnobSummary() << endl;
    return 1;
}

/*
 * A routine to report value (signed and hex-formatted) and width of an immediate operand.
 */
static void OnInstruction(ADDRINT unsigned_value, long signed_value, bool is_signed, INT32 length_bits)
{
    *outFile << "Contains an immediate value: " << "0x" << hex << setfill('0') << setw(length_bits/4)
             << unsigned_value << setfill(' ') << setw(1) << dec;
    if (is_signed)
    {
        *outFile << " (signed: " << signed_value << "),";
    }
    *outFile << " operand is " << length_bits << " bits long." << endl;
}

/*
 * A routine to query the immediate operand per instruction. May be called at
 * instrumentation or analysis time.
 */
VOID GetOperLenAndSigned(INS ins, INT32 i, INT32& length_bits, bool& is_signed)
{
    xed_decoded_inst_t* xedd = INS_XedDec(ins);
    // To print out the gory details uncomment this:
    // char buf[2048];
    // xed_decoded_inst_dump(xedd, buf, 2048);
    // *outFile << buf << endl;
    length_bits = 8*xed_decoded_inst_operand_length(xedd, i);
    is_signed = xed_decoded_inst_get_immediate_is_signed(xedd);
}

/*
 * Instrumentation-time routine inspecting a single instruction, looking for
 * those with an immediate operand.
 */
VOID Instruction(INS ins, VOID *v)
{
    *outFile << "Querying instruction w/opcode: " << INS_Mnemonic(ins) << endl;
    // Go over operands
    INT32 count = INS_OperandCount(ins);
    bool operands_reported = false;
    for (INT32 i = 0; i < count; i++)
    {
        
        if (INS_OperandIsImmediate(ins, i))
        {
            // Get the value itself
            ADDRINT value = INS_OperandImmediate(ins, i);
            long signed_value = (long)value;
            // Get length information
            INT32 length_bits = -1;
            bool is_signed = false;
            GetOperLenAndSigned(ins, i, length_bits, is_signed);
            OnInstruction(value, signed_value, is_signed, length_bits);
            operands_reported = true;
        }
    }
    if (!operands_reported)
    {
        *outFile << "No immediate operands per this command" << endl;
    }
}

/*
 * Instrumentation-time routine looking for the routine we'd like to instrument.
 */
static VOID ImageLoad(IMG img, VOID * v)
{
    if (IMG_IsMainExecutable(img))
    {
        *outFile << "In main application image" << endl;
        // Search for the assembly routine in the application
        RTN AsmRtn = RTN_FindByName(img, "operImmCmds");
        if (!RTN_Valid(AsmRtn))
        {
            AsmRtn = RTN_FindByName(img, "_operImmCmds");
        }
        if (RTN_Valid(AsmRtn))
        {
            *outFile << "Function operImmCmds found" << endl;
            RTN_Open(AsmRtn);
            // Go over each of the routine's instructions
            for (INS ins = RTN_InsHead(AsmRtn); INS_Valid(ins); ins = INS_Next(ins))
            {
                Instruction(ins, 0);
            }
            RTN_Close(AsmRtn);
            *outFile << "Done with function operImmCmds" << endl;
        }
        else
        {
            *outFile << "Function operImmCmds not found!" << endl;
        }
    }
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
    if( PIN_Init(argc,argv) )
        return Usage();
    // Next call is needed, otherwise we can't find routines by name
    PIN_InitSymbols();
    outFile = KnobOutputFile.Value().empty() ? &cout : new std::ofstream(KnobOutputFile.Value().c_str());
    IMG_AddInstrumentFunction(ImageLoad, 0);
    PIN_StartProgram();    // Never returns
    return 0;
}
