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
#ifndef XSTATE_BV_UTILS_H
#define XSTATE_BV_UTILS_H

#include <string>

using std::string;


/////////////////////
// TYPE DEFINITIONS
/////////////////////

enum TEST_REG_CLASS
{
    TEST_REG_CLASS_X87 = 0,
    TEST_REG_CLASS_SSE,
    TEST_REG_CLASS_AVX,
    TEST_REG_CLASS_SIZE,
    TEST_REG_CLASS_INVALID = TEST_REG_CLASS_SIZE
};


extern "C"
{

/////////////////////
// GLOBAL VARIABLES
/////////////////////


extern const unsigned int testRegSize[TEST_REG_CLASS_SIZE];
extern const unsigned int testRegLocation[TEST_REG_CLASS_SIZE];
extern const unsigned char xstateBvMasks[TEST_REG_CLASS_SIZE];
extern const string componentStrings[TEST_REG_CLASS_SIZE];
extern const unsigned char* toolRegisterValues[TEST_REG_CLASS_SIZE];


/////////////////////
// UTILITY FUNCTIONS
/////////////////////

extern TEST_REG_CLASS GetTestReg(const string& arg);

} // extern "C"

#endif // XSTATE_BV_UTILS_H
