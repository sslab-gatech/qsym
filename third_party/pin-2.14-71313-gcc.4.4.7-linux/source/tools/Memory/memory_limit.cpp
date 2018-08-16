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
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <assert.h>
#include "pin.H"

// This PIN tool shall check PIN's memory limit knob
//

#define ALLOC_CHUNK 0x10000

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

//Output file where to write everything
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,    "pintool",
    "o", "memory_limit.out", "specify output file name");

//have memory addresses within that region
KNOB<ADDRINT> KnobBytesToAllocate(KNOB_MODE_WRITEONCE,    "pintool",
    "b", "0x10000", "Number of bytes to allocate");


/* ===================================================================== */
/* Globals */
/* ===================================================================== */

//Output file
FILE * out;

VOID OutOfMemory(size_t sz, VOID* arg)
{
    fprintf(out, "Failed to allocate dynamic memory: Out of memory!\n");
    fclose(out);
    exit(3);
}

VOID* AllocateAndCheck(long size)
{
    void* p = malloc((size_t)size);
    if (NULL == p)
    {
        fprintf(out, "Failed to allocate dynamic memory with size %lx.\n", size);
        fclose(out);
        exit(1);
    }

    return p;
}

// This function allocates number of bytes specified by the -b knobs.
// The bytes are allocated in chunks of size ALLOC_CHUNK.
VOID AppStart(VOID *v)
{
    fprintf(out, "Allocating total %lx bytes.\n", (long)KnobBytesToAllocate.Value());
    long count = (long)KnobBytesToAllocate.Value() / ALLOC_CHUNK;
    long remainder = (long)KnobBytesToAllocate.Value() % ALLOC_CHUNK;
    for (long i = 0; i < count; i++)
    {
        void* p = AllocateAndCheck(ALLOC_CHUNK);
        fprintf(out, "Iteration %lx, returned: %p\n", i, p);
    }
    if (remainder > 0)
    {
        void* p = AllocateAndCheck(remainder);
        fprintf(out, "Last iteration, returned: %p\n", p);
    }

    fprintf(out, "Test unexpectedly passed (it should fail).\n");
    fclose(out);
}

int main(int argc, char * argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    out = fopen(KnobOutputFile.Value().c_str(), "w");

    PIN_AddOutOfMemoryFunction(OutOfMemory, NULL);

    PIN_AddApplicationStartFunction(AppStart, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
