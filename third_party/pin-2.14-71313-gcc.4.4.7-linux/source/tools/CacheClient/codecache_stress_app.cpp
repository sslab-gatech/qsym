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
 * codecache_stress_app.cpp
 *
 *  Created on: Apr 13, 2014
 *      Author: bnirenbe
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#ifdef TARGET_WINDOWS
#   include <windows.h>
#else
#   include <sys/mman.h>
#   include <errno.h>
#endif

extern "C" void* getStartNop();
extern "C" void* getEndNop();
extern "C" void* getBeforeFunc();
extern "C" void* getAfterFunc();

static int NopsCount = -1;

void* AllocateCodeRegion(int size)
{
    void* res;
#ifdef TARGET_WINDOWS
    res = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (NULL == res)
    {
        fprintf(stderr, "Failed to allocate %d bytes (GetLastError=%u)\n", size, GetLastError());
        exit(1);
    }
#else
    res = mmap(NULL, size, PROT_WRITE | PROT_READ | PROT_EXEC, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (MAP_FAILED == res)
    {
        fprintf(stderr, "Failed to allocate %d bytes (errno=%d)\n", size, errno);
        exit(1);
    }
#endif
    return res;
}

void GenerateAndRunCode(int nops)
{
    void* startNop = getStartNop();
    void* endNop = getEndNop();
    void* beforeFunc = getBeforeFunc();
    void* afterFunc = getAfterFunc();
    void* codeToExecuteNext = NULL;
    void* newCode = NULL;

    printf("\tWill generate and run program with %d nops...\n", nops);

    int nopSize = (int)((char*)endNop - (char*)startNop);
    int funcSize = (int)((char*)afterFunc - (char*)beforeFunc);
    newCode = AllocateCodeRegion(nopSize*nops + funcSize);
    char* codePtr = (char*)newCode;
    for (int i = 0; i < nops; i++, codePtr+= nopSize)
    {
        memcpy(codePtr, startNop, nopSize);
    }
    codeToExecuteNext = (void*)codePtr;
    memcpy(codePtr, beforeFunc, (size_t)funcSize);

    while (codeToExecuteNext >= newCode)
    {
        void (*fn)() = (void (*)())codeToExecuteNext;
        (*fn)();
        codeToExecuteNext = (void*)((char*)codeToExecuteNext - nopSize);
    }
}

int ParseCommandLine(int argc, const char* argv[])
{
    if (2 != argc)
    {
        return 0;
    }
    if (0 == sscanf(argv[1], "%d", &NopsCount) || NopsCount < 0)
    {
        fprintf(stderr, "Bad NOPs count: %s\n", argv[1]);
        exit(1);
    }
    return 1;
}

int main(int argc, const char* argv[])
{
    if (!ParseCommandLine(argc, argv))
    {
        printf("Usage: %s <NOPs count>\n", argv[0]);
        return 2;
    }
    GenerateAndRunCode(NopsCount);
}
