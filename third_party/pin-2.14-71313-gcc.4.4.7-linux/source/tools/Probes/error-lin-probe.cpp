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
//
// This tool demonstrates how to get the value of the application's
// errno on linux in probe mode.
//
// 
// There are symbols called __errno_location in both libc and libpthread.
// This test captures the address of the __errno_location() function in
// one of the libraries and not in the other.   Today, the implementations
// in libc and libpthread point to the same actual location (in a TLS),
// but this assumption is fragile and may change at some time int he future.
//
// In Android, the function is called __errno. Pthreads is baked into bionic,
// So we don't have the same issues that we have in Linux.
//

#include "pin.H"
#include <iostream>
#include <stdlib.h>
#include <errno.h>

using namespace std;

#if defined(TARGET_ANDROID)
#define ERRNO_SYMBOL ("__errno")
#else
#define ERRNO_SYMBOL ("__errno_location")
#endif

typedef ADDRINT * (*ERRNO_LOCATION_FUNPTR)();
ADDRINT pf_errno_location = 0;


/* ===================================================================== */
VOID ToolCheckError()
{
    if ( pf_errno_location != 0 )
    {
        ADDRINT * err_loc = (*(ERRNO_LOCATION_FUNPTR)pf_errno_location)();
        
        int err_value = *err_loc;
        
        cerr << "Tool: errno=" << err_value << endl;
    }
    else
        cerr << "Tool: __errno_location() not found." << endl;
    
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID *v)
{

    RTN errno_location_rtn = RTN_FindByName(img, ERRNO_SYMBOL);
    if (RTN_Valid(errno_location_rtn))
    {
        pf_errno_location = RTN_Address(errno_location_rtn);
    }
    
    if ( IMG_IsMainExecutable( img ))
    {
        PROTO proto = PROTO_Allocate( PIN_PARG(void), CALLINGSTD_DEFAULT,
                                      "CheckError", PIN_PARG_END() );
        
        RTN rtn = RTN_FindByName(img, "CheckError");
        if (RTN_Valid(rtn) && RTN_IsSafeForProbedReplacement(rtn))
        {
            RTN_ReplaceSignatureProbed(rtn, AFUNPTR(ToolCheckError),
                                       IARG_PROTOTYPE, proto,
                                       IARG_END);
        }    
        PROTO_Free( proto );
    }
}

/* ===================================================================== */
int main(INT32 argc, CHAR *argv[])
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);
    
    PIN_StartProgramProbed();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */


