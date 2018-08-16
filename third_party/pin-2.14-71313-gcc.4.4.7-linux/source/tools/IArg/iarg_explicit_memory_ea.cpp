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
 * This tool should instrument iarg_explicit_memory_ea_app.
 * First, this tool instrument every instuction with explicit memory operand with IARG_INST_PTR
 * and IARG_EXPLICIT_MEMORY_EA and then records these instructions when the application executes them.
 * Afterward, the application calls checkVar() (which is instrumented by this tool)
 * specifying some important instructions that was executed, providing their expected memory operand
 * value and instruction pointer.
 * This tool then checks that it managed to record these important instructions
 * and that it managed to catch the correct memory operand vaoues for the instruction
 * using IARG_EXPLICIT_MEMORY_EA.
  */
#include "pin.H"
#include <fstream>
#include <iostream>
#include <set>
#include <utility>
#include <cstdlib>

#ifdef TARGET_MAC
#define NAME(x) "_" x
#else
#define NAME(x) x
#endif

using namespace std;

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,        "pintool",
    "o", "iarg_explicit_memory_ea.out", "specify output file name");

static ofstream* out = NULL;
static set<pair<ADDRINT,ADDRINT> > memOpAddresses;

/* =====================================================================
 * Called upon bad command line argument
 * ===================================================================== */
INT32 Usage()
{
    cerr <<
        "This pin tool instruments memory instructions with explicit operand, "
            <<" so the address the're refering to will be printed\n"
        "\n";

    cerr << KNOB_BASE::StringKnobSummary();

    cerr << endl;

    return -1;
}

/* =====================================================================
 * Called upon program finish
 * ===================================================================== */
VOID Fini(int, VOID * v)
{
    *out << "Fini" << endl;
}

/* =====================================================================
 * The analysis routine that is instrumented before any memory operand instruction
 * ===================================================================== */
VOID MemOpAnalysis(ADDRINT pc, ADDRINT addr)
{
    memOpAddresses.insert(pair<ADDRINT,ADDRINT>(pc, addr));
    *out << hex << "At PC="<< pc << " Memory operation with address " << addr << endl;
}

/* =====================================================================
 * Iterate over a trace and instrument its memory related instructions
 * ===================================================================== */
VOID Trace(TRACE trace, VOID *v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            if (INS_HasExplicitMemoryReference(ins))
            {
                INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(MemOpAnalysis),
                        IARG_INST_PTR, IARG_EXPLICIT_MEMORY_EA, IARG_END);
            }
        }
    }
}

/* =====================================================================
 * This function should replace the checkVar function of the application
 * ===================================================================== */
void CheckVarReplaced(const char* name, void* pc, void* value)
{
    *out << "CheckVar called for " << name << ", value " << value << " at address " << pc << endl;
    pair<ADDRINT,ADDRINT> p((ADDRINT)pc,(ADDRINT)value);
    if (memOpAddresses.end() == memOpAddresses.find(p))
    {
        *out << "Instruction for " << name << " at " << pc << " with operand " << value
                << " wasn't caught in instrumentation" << endl;
        out->flush();
        exit(4);
    }
}

/* =====================================================================
 * Called upon image load to instrument the function checkVar
 * ===================================================================== */
VOID ImageLoad(IMG img, VOID *v)
{
    if (IMG_IsMainExecutable(img))
    {
        RTN rtn = RTN_FindByName(img, NAME("checkVar"));
        if (!RTN_Valid(rtn))
        {
            *out << "Cannot find routine " << NAME("checkVar") << " in main image" << endl;
            out->flush();
            exit(3);
        }
        RTN_Replace(rtn, AFUNPTR(CheckVarReplaced));
    }
}

/* =====================================================================
 * Entry point for the tool
 * ===================================================================== */
int main(int argc, CHAR *argv[])
{
    PIN_InitSymbols();
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }
    out = new std::ofstream(KnobOutputFile.Value().c_str());
    IMG_AddInstrumentFunction(ImageLoad, 0);
    TRACE_AddInstrumentFunction(Trace, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
