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
#include <fstream>
#include <cstring>
#include <cassert>
#include "xstateBVUtils.h"
#include "../Utils/regvalue_utils.h"

using std::ofstream;


/////////////////////
// GLOBAL VARIABLES
/////////////////////

// A knob for defining which context to test. One of:
// 1. context   - regular CONTEXT passed to the analysis routine using IARG_CONTEXT.
// 2. partial   - partial CONTEXT passed to the analysis routine using IARG_PARTIAL_CONTEXT.
// 3. reference - IARG_REG_REFERENCE passed to the analysis routine.
KNOB<string> KnobModificationMethod(KNOB_MODE_WRITEONCE, "pintool",
    "method", "context", "specify which context to test. One of default|partial.");

// A knob for defining which state class to test. One of:
// 1. x87   - The x87 FPU registers.
// 2. SSE   - The SSE registers (xmms).
// 3. AVX   - The AVX registers (ymms).
KNOB<string> KnobTestStateClass(KNOB_MODE_WRITEONCE, "pintool",
    "stateClass", "x87", "specify which context to test. One of x87|SSE|AVX (case sensitive).");

// A knob for defining the output file name
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "checkXStateBV_tool.out", "specify output file name");

// ofstream object for handling the output
ofstream OutFile;

REG testedRegister = REG_INVALID();

TEST_REG_CLASS regClass = TEST_REG_CLASS_INVALID;


/////////////////////
// UTILITY FUNCTIONS
/////////////////////

REG GetRegister()
{
    static const REG testRegs[TEST_REG_CLASS_SIZE] =
    {
        REG_FPCW,
        REG_XMM3,
        REG_YMM3
    };

    return testRegs[regClass];
}


/////////////////////
// ANALYSIS FUNCTIONS
/////////////////////

static void ChangeRegister(CONTEXT * ctxt)
{
    PIN_REGISTER val;
    const UINT64* newval = reinterpret_cast<const UINT64*>(toolRegisterValues[regClass]);
    AssignNewPinRegisterValue(&val, newval, (testRegSize[regClass]+7)/8);
    PIN_SetContextRegval(ctxt, testedRegister, reinterpret_cast<UINT8*>(&val));
}


static void ChangeRegisterAndExecute(CONTEXT * ctxt, ADDRINT executeAtAddr)
{
    ChangeRegister(ctxt);
    PIN_SetContextReg(ctxt, REG_INST_PTR, executeAtAddr);
    PIN_ExecuteAt(ctxt);
}


static void ChangeRegisterReference(PIN_REGISTER* reg)
{
    const UINT64* newval = reinterpret_cast<const UINT64*>(toolRegisterValues[regClass]);
    AssignNewPinRegisterValue(reg, newval, (testRegSize[regClass]+7)/8);
}


/////////////////////
// CALLBACKS
/////////////////////

static VOID ImageLoad(IMG img, VOID * v)
{
    if (IMG_IsMainExecutable(img))
    {
        // Find the placeholder for PIN_ExecuteAt
        RTN executeAtRtn = RTN_FindByName(img, "ExecuteAt");
        assert(RTN_Valid(executeAtRtn));

        // Find and application's ChangeRegisterValue function.
        RTN changeRegisterValueRtn = RTN_FindByName(img, "ChangeRegisterValue");
        assert(RTN_Valid(changeRegisterValueRtn));
        RTN_Open(changeRegisterValueRtn);
        if (KnobModificationMethod.Value() == "context")
        {
            RTN_InsertCall(changeRegisterValueRtn, IPOINT_BEFORE, AFUNPTR(ChangeRegisterAndExecute),
                                                                  IARG_CONTEXT,
                                                                  IARG_ADDRINT, RTN_Address(executeAtRtn),
                                                                  IARG_END);
        }
        else if (KnobModificationMethod.Value() == "partial")
        {
            REGSET regsin;
            REGSET regsout;
            REGSET_Clear(regsin);
            REGSET_Clear(regsout);
            REGSET_Insert(regsout, testedRegister);
            RTN_InsertCall(changeRegisterValueRtn, IPOINT_BEFORE, AFUNPTR(ChangeRegister),
                                                                  IARG_PARTIAL_CONTEXT, &regsin, &regsout,
                                                                  IARG_END);
        }
        else if (KnobModificationMethod.Value() == "reference")
        {
            RTN_InsertCall(changeRegisterValueRtn, IPOINT_BEFORE, AFUNPTR(ChangeRegisterReference),
                                                                  IARG_REG_REFERENCE, GetRegister(),
                                                                  IARG_END);
        }
        else
        {
            OutFile << "ERROR: Unknown modification method specified: " << KnobModificationMethod.Value() << endl;
            PIN_ExitApplication(12); // never returns
        }
        RTN_Close(changeRegisterValueRtn);
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
    // Initialize Pin
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    // Open the output file
    OutFile.open(KnobOutputFile.Value().c_str());

    // Set the test parameters
    regClass = GetTestReg(KnobTestStateClass.Value());
    if (TEST_REG_CLASS_INVALID == regClass)
    {
        OutFile << "ERROR: Unknown state class: " << KnobTestStateClass.Value() << endl;
        PIN_ExitApplication(11); // never returns
    }
    testedRegister = GetRegister();

    // Add instrumentation
    IMG_AddInstrumentFunction(ImageLoad, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Start running the application
    PIN_StartProgram(); // never returns

    return 0;
}
