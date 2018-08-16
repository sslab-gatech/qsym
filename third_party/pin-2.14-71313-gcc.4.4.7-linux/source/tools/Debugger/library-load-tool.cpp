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
 * This tool tests GDB's svr4 libraries extension.
 * One of the tests involves making GDB think that libc.so was loaded at
 * base address 0xd00dead and then check in GDB output that it really
 * shows that libc.so was loaded there.
 */
#include <pin.H>
#include <iostream>
#include "link.h"
#include "stdlib.h"
#include "string.h"

using namespace std;

/* =====================================================================
 * Called upon bad command line argument
 * ===================================================================== */
INT32 Usage()
{
    std::cerr <<
        "This pin tool tests GDB svr4 libraries extension" << std::endl;

    std::cerr << KNOB_BASE::StringKnobSummary();

    std::cerr << std::endl;

    return -1;
}

VOID Image(IMG img, VOID *)
{
    LINUX_LOADER_IMAGE_INFO* oldLi = (LINUX_LOADER_IMAGE_INFO*)IMG_GetLoaderInfo(img);
    if (oldLi != NULL)
    {
        // Make sure IMG_LowAddress is the same as the loader base address
        ASSERT(IMG_LoadOffset(img) == oldLi->l_addr, hexstr(IMG_LoadOffset(img)) + " != " + hexstr(oldLi->l_addr));
    }
    if (IMG_Name(img).find("libc.so") != string::npos)
    {
        struct link_map* lm = (struct link_map*)malloc(sizeof(struct link_map));
        string name_copy = IMG_Name(img);
        // Base address as appear in ELF file before relocating the image
        ADDRINT suggestedBaseAddress = IMG_LowAddress(img) - oldLi->l_addr;
        LINUX_LOADER_IMAGE_INFO li;
        memset(lm, 0, sizeof(*lm));
        // Make libc.so appear to be loaded at 0xd00dead
        // GDB should see libc.so loaded at that address
        lm->l_addr = 0xd00dead - suggestedBaseAddress;
        lm->l_ld = (ElfW(Dyn)*)(oldLi->l_ld - IMG_LowAddress(img) + 0xd00dead);
        lm->l_name = (char*)name_copy.c_str();
        lm->l_next = ((struct link_map*)oldLi->lm)->l_next;
        lm->l_prev = ((struct link_map*)oldLi->lm)->l_prev;

        li.lm = (ADDRINT)lm;
        li.l_ld = (ADDRINT)lm->l_ld;
        li.l_addr = lm->l_addr;
        li.name = lm->l_name;
        IMG_SetLoaderInfo(img, &li);
    }
}

/* =====================================================================
 * Entry point for the tool
 * ===================================================================== */
int main(int argc, char * argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }
    IMG_AddInstrumentFunction(Image, NULL);

    // Never returns
    PIN_StartProgram();
    return 0;
}
/* ===================================================================== */
/* eof */
/* ===================================================================== */
