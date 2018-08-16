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
// This tool instruments the modifyFlags() function in the application
// then get and get the flag register.
// The tool is used to verify the correctness of passing the flag
// register by reference to an analysis routine.

#include <iostream>
#include <fstream>
#include <pin.H>

#ifdef TARGET_MAC
#define GLOBALFUN_NAME(name) "_" name
#else
#define GLOBALFUN_NAME(name) name
#endif

using std::ofstream;


/////////////////////
// GLOBAL VARIABLES
/////////////////////

// A knob for defining the output file name
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "reg_reference_flags.out",
                            "specify file name for reg_reference_flags output");

// A knob for defining which register reference to use. One of:
// 1. default   - regular REG_REFERENCE passed to the analysis routine using IARG_REG_REFERENCE.
// 2. const     - const REG_REFERENCE passed to the analysis routine using IARG_REG_CONST_REFERENCE.
KNOB<string> KnobTestReference(KNOB_MODE_WRITEONCE, "pintool",
    "testreference", "default", "specify which context to test. One of default|const.");

// ofstream object for handling the output.
ofstream OutFile;


/////////////////////
// UTILITY FUNCTIONS
/////////////////////

static int Usage()
{
    cerr << "This tool verifies the correctness of passing the flags register by reference to an analysis routine." <<
            endl << endl << KNOB_BASE::StringKnobSummary() << endl;
    return 1;
}


/////////////////////
// ANALYSIS FUNCTIONS
/////////////////////

static void ChangeRegBefore(ADDRINT* val)
{
    OutFile << "Flags: " << std::hex << *val << std::endl;
    if (KnobTestReference.Value() == "default")
    {
        *val = 0xcd0;
    }
}


/////////////////////
// CALLBACKS
/////////////////////

static VOID ImageLoad(IMG img, VOID * v)
{
    if (IMG_IsMainExecutable(img))
    {
        RTN modifyFlagsRtn = RTN_FindByName(img, GLOBALFUN_NAME("modifyFlags"));
        ASSERTX(RTN_Valid(modifyFlagsRtn));
        RTN_Open(modifyFlagsRtn);
        INS ins = RTN_InsHeadOnly(modifyFlagsRtn);
        if (KnobTestReference.Value() == "default")
        {
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(ChangeRegBefore), IARG_REG_REFERENCE, REG_AppFlags(), IARG_END);
        }
        else if (KnobTestReference.Value() == "const")
        {
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(ChangeRegBefore), IARG_REG_CONST_REFERENCE, REG_AppFlags(), IARG_END);
        }
        else
        {
            OutFile << "ERROR: Unknown reference requested for testing: " << KnobTestReference.Value() << endl;
            PIN_ExitApplication(2); // never returns
        }
        RTN_Close(modifyFlagsRtn);
    }
}

static VOID Fini(INT32 code, VOID *v)
{
    OutFile.close();
}


/////////////////////
// MAIN FUNCTION
/////////////////////

int main(int argc, char * argv[])
{
    // Initialize pin
    PIN_InitSymbols();
    if (PIN_Init(argc, argv)) return Usage();

    OutFile.open(KnobOutputFile.Value().c_str());

    // Register Instruction to be called to instrument instructions
    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
