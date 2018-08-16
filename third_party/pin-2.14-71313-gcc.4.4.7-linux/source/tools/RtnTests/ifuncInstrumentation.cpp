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
/*! @file */

#include <iostream>
#include <sys/time.h>
#include "pin.H"

string rtn_name;

KNOB<string> KnobFunctionNameToInstrument(KNOB_MODE_WRITEONCE, "pintool", "function_name", "gettimeofday",
                    "function name to instrument" );

int NewImplementation(const char * ptr1, const char *ptr2) {
    cout << "New implementation!" << endl;
    return 0;
}

void BeforeResolverFunction(void *img_name, ADDRINT rtn_addr)
{
    cout << "Hello once (resolver)" << endl;
}

VOID ImageLoad( IMG img, VOID *v )
{
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            if (RTN_Name(rtn).compare(rtn_name) == 0 ||
                RTN_Name(rtn).compare("__" + rtn_name) == 0) {
                if (!SYM_IFuncResolver(RTN_Sym(rtn)))
                        continue;
                
                cout << "Found " << RTN_Name(rtn).c_str() << " in " << IMG_Name(img);
                RTN resolver = rtn;
                RTN impl = RTN_IFuncImplementation(rtn);

                cout << "...  Replacing" << endl;

                ASSERTX(RTN_Valid(resolver));
                ASSERTX(RTN_Valid(impl));

                RTN_Open(impl);
                RTN_InsertCall(impl, IPOINT_BEFORE, AFUNPTR(NewImplementation), IARG_END);
                RTN_Close(impl);

                // Instrumenting the resolver function, should be called once
                RTN_Open(resolver);
                RTN_InsertCall(resolver, IPOINT_BEFORE, AFUNPTR(BeforeResolverFunction),
                        IARG_PTR, IMG_Name(img).c_str(),
                        IARG_ADDRINT, RTN_Address(rtn),
                        IARG_END);
                RTN_Close(resolver);

            }
        }
    }
}

int main (INT32 argc, CHAR *argv[])
{
    // Initialize pin
    //
    if (PIN_Init(argc, argv)) return 0;

    // Initialize symbol processing
    //
    PIN_InitSymbolsAlt(IFUNC_SYMBOLS);

    //Initialize global variables
    rtn_name = KnobFunctionNameToInstrument.Value();
    cout << "rtn_name : " << rtn_name << endl;
    

    // Register ImageLoad to be called when an image is loaded
    //
    IMG_AddInstrumentFunction( ImageLoad, 0 );
    
    // Start the program, never returns
    //
    PIN_StartProgram();
    
    return 0;
}
