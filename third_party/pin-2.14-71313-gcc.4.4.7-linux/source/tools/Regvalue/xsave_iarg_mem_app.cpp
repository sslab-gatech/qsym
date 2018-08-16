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
#include <cstring>
#include <cstdio>

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

int main(int argc, const char* argv[])
{
    memset(xsaveArea, 0, sizeof(xsaveArea));
    unsigned char before[32] = { 0 }; // empty buffer large enough to hold any context register up to AVX.
    unsigned char after[32] = { 0 }; // empty buffer large enough to hold any context register up to AVX.
    DoXsave(); // get the register value before the change
    printf("XSAVE on %p\n", (void*)xsaveArea);
    return 0;
}
