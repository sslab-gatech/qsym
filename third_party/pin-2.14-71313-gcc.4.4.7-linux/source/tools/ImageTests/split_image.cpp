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
/**
 * This tool writes the region/s of every image loaded during the lifetime
 * of the application.
 * This information is then compared with the actual load addresses of the 
 * segments as the application views them (split_image_app).
 * The comparison is done using the python script region_compare.py.
 */

#include <cstdio>
#include <cstdlib>
#include "pin.H"


FILE *fp = 0;

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "split_image.out", "specify output file name");


// check if each image area is really covered by its region
VOID ImageLoad (IMG img, VOID *v)
{
    if (IMG_NumRegions(img) == 1)
    {
        fprintf(fp, "%s, %p-%p\n", IMG_Name(img).c_str(), Addrint2VoidStar(IMG_LowAddress(img)), Addrint2VoidStar(IMG_HighAddress(img)));
    }
    else
    {
        for(int i = 0; i < IMG_NumRegions(img); i++)
        {
            ADDRINT high = IMG_RegionHighAddress(img, i);
            ADDRINT low = IMG_RegionLowAddress(img, i);
            fprintf(fp, "%s, %p-%p\n", IMG_Name(img).c_str(), Addrint2VoidStar(low), Addrint2VoidStar(high));
        }
    }
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    fclose(fp);
}

int main(int argc, char * argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    fp = fopen(KnobOutputFile.Value().c_str(), "w");
    if (fp == NULL) {
        fprintf(stderr, "Couldn't open %s for output\n", KnobOutputFile.Value().c_str());
        exit(1);
    }

    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}


