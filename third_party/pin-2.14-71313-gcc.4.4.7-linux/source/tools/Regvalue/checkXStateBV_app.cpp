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
 *  This application checks that changes made to the x87, sse and avx registers by the tool, before any application
 *  modification was made, are in fact visible.
 *  Since the application does not change these registers, the appropriate bits in the XSTATE_BV save mask are cleared.
 *  Pin must set the necessary bits in order for the tool's changes to take effect upon xrstor.
 *
 *  Usage: CheckXStateBV <component>
 *  where component is one of:
 *      x87 - test the x87 state component
 *      SSE - test the sse state component
 *      AVX - test the avx state component
 */

#include <iostream>
#include <cstring>
#include "xstateBVUtils.h"

using std::cerr;
using std::endl;


#ifdef TARGET_WINDOWS
# define ASMNAME(name)
# define ALIGN64 __declspec(align(64))
#else
# define ASMNAME(name) asm(name)
# define ALIGN64 __attribute__ ((aligned (64)))
#endif


/////////////////////
// EXTERNAL FUNCTIONS
/////////////////////

extern "C" void DoXsave() ASMNAME("DoXsave");


/////////////////////
// GLOBAL VARIABLES
/////////////////////

extern "C"
{
unsigned char ALIGN64 xsaveArea[832] ASMNAME("xsaveArea");
}


/////////////////////
// UTILITY FUNCTIONS
/////////////////////

static bool IsOn(TEST_REG_CLASS regClass)
{
    const unsigned char xstateBv = xsaveArea[512];
    return (xstateBv & xstateBvMasks[regClass]);
}


static void GetRegValue(unsigned char* buf, TEST_REG_CLASS regClass)
{
    unsigned int size = testRegSize[regClass];
    if (TEST_REG_CLASS_AVX == regClass)
    {
        // First get the upper half
        regClass = TEST_REG_CLASS_SSE;
        size = testRegSize[regClass];
        memcpy(buf + size, &xsaveArea[testRegLocation[TEST_REG_CLASS_AVX]], size);
        // Now get the lower half (xmm)
    }
    memcpy(buf, &xsaveArea[testRegLocation[regClass]], testRegSize[regClass]);
}


// This is a placeholder for the tool to change the register values.
extern "C" void ChangeRegisterValue() ASMNAME("ChangeRegisterValue");
void ChangeRegisterValue()
{
    // does nothing
}


// This is a placeholder for the tool to start execution after changing the register.
extern "C" void ExecuteAt() ASMNAME("ExecuteAt");
void ExecuteAt()
{
    // does nothing
}


/////////////////////
// MAIN FUNCTION
/////////////////////

/*
 * Expected arguments:
 * [1]  - component
 */
int main(int argc, const char* argv[])
{
    // Verify one command line argument - component
    if (argc != 2)
    {
        cerr << "ERROR: Expected 1 command line argument, got " << argc-1 << ". Aborting test." << endl;
        return 1;
    }

    // Identify the requested state component.
    TEST_REG_CLASS regClass = GetTestReg(argv[1]);
    if (TEST_REG_CLASS_INVALID == regClass)
    {
        cerr << "ERROR: Unknown state component '" << argv[1] << "'. Aborting test." << endl;
        return 2;
    }

    // Perform the test.
    unsigned char before[32] = { 0 }; // empty buffer large enough to hold any context register up to AVX.
    unsigned char after[32] = { 0 }; // empty buffer large enough to hold any context register up to AVX.
    DoXsave(); // get the register value before the change
    if (IsOn(regClass))
    {
        cerr << "WARNING: The " << componentStrings[regClass] << " state bit was expected to be clear at this point "
             << "in the test but it was set." << endl;
    }
    GetRegValue(before, regClass);

    ChangeRegisterValue(); // allow the tool to change the register value.
    ExecuteAt();

    bool success = true;
    DoXsave(); // get the register value before the change
    if (!IsOn(regClass))
    {
        cerr << "ERROR: The " << componentStrings[regClass] << " state bit was expected to be set at this point "
             << "in the test but it was clear." << endl;
        success = false;
    }
    GetRegValue(after, regClass);
    if (0 == memcmp(before, after, testRegSize[regClass]))
    {
        cerr << "ERROR: Error while testing the " << componentStrings[regClass] << " state. The values before and after "
             << "the test were the same." << endl;
        success = false;
    }
    if (0 != memcmp(after, toolRegisterValues[regClass], testRegSize[regClass]))
    {
        cerr << "ERROR: Error while testing the " << componentStrings[regClass] << " state. The value after the tool's change "
             << "did not match the expected value." << endl;
        success = false;
    }

    if (!success) return 3;
    return 0;
}
