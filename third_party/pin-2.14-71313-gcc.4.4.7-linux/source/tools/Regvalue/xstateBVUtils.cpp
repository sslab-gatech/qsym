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
#include "xstateBVUtils.h"


const unsigned int testRegSize[TEST_REG_CLASS_SIZE] =
{
    /* TEST_REG_CLASS_X87 */    2,  // fcw
    /* TEST_REG_CLASS_SSE */    16, // xmm3
    /* TEST_REG_CLASS_AVX */    32, // ymm3
};


const unsigned int testRegLocation[TEST_REG_CLASS_SIZE] =
{
    /* TEST_REG_CLASS_X87 */    0,      // fcw
    /* TEST_REG_CLASS_SSE */    208,    // xmm3
    /* TEST_REG_CLASS_AVX */    624,    // ymm3 (upper 128 bits)
};


// Masks for checking if a single state component in the XSTATE_BV state-component bitmaps is set.
const unsigned char xstateBvMasks[TEST_REG_CLASS_SIZE] =
{
    0x01,   // bit 0 in the XSTATE_BV save mask - controls the X87 registers
    0x02,   // bit 1 in the XSTATE_BV save mask - controls the SSE registers
    0x04    // bit 2 in the XSTATE_BV save mask - controls the AVX registers
};


const string componentStrings[TEST_REG_CLASS_SIZE] =
{
    "x87",
    "SSE",
    "AVX"
};


static const unsigned char fpcwval[] = { 0x4e, 0x1f };
static const unsigned char xmm3val[] = { 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
                                         0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc };
static const unsigned char ymm3val[] = { 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
                                         0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
                                         0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
                                         0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd };


const unsigned char* toolRegisterValues[TEST_REG_CLASS_SIZE] =
{
    fpcwval,
    xmm3val,
    ymm3val
};


TEST_REG_CLASS GetTestReg(const string& arg)
{
    for (unsigned int i = 0; i < TEST_REG_CLASS_SIZE; ++i)
    {
        if (componentStrings[i] == arg) return (TEST_REG_CLASS)i;
    }
    return TEST_REG_CLASS_INVALID;
}
