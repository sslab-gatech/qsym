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
 * This tool tests the retrival of a register, specified at command line, using the buffering API.
 * 
 */
#include "pin.H"
#include <iostream>
#include <stdio.h>
#include <stddef.h>
#include <cstdlib>

BUFFER_ID bufId;
TLS_KEY buf_key;
REG reg_to_check;

#define NUM_BUF_PAGES 8192

/*
 * Register to test
 */
KNOB<string> KnobRegister(KNOB_MODE_WRITEONCE, "pintool", "r", "xmm0", "register to test its retrieve");

/*!
 *  Print out help message.
 */
INT32 Usage()
{
    cerr << "This tool contains errors.  Do not use!" << endl;

    return -1;
}

VOID Trace(TRACE trace, VOID *v)
{
    for(BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl=BBL_Next(bbl)){
        for(INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins=INS_Next(ins)){
            // This is an error test!!!
            // It is expected that 'reg_to_check' cannot be retrieved using IARG_REG_VALUE
            INS_InsertFillBuffer(ins, IPOINT_BEFORE, bufId,
                    IARG_REG_VALUE, reg_to_check, 0,
                    IARG_END);
        }
    }
}

/*!
 * Called when a buffer fills up, or the thread exits, so we can process it or pass it off
 * as we see fit.
 * @param[in] id		buffer handle
 * @param[in] tid		id of owning thread
 * @param[in] ctxt		application context when the buffer filled
 * @param[in] buf		actual pointer to buffer
 * @param[in] numElements	number of records
 * @param[in] v			callback value
 * @return  A pointer to the buffer to resume filling.
 */
VOID * BufferFull(BUFFER_ID bid, THREADID tid, const CONTEXT *ctxt, VOID *buf,
                  UINT64 numElements, VOID *v)
{
    return buf;
}

void ThreadStart(THREADID tid, CONTEXT * context, int flags, void * v)
{
    // We check that we got the right thing in the buffer full callback
    PIN_SetThreadData(buf_key, PIN_GetBufferPointer(context, bufId), tid);
}

REG FindRegToTest()
{
    string reg_string = KnobRegister;
    for (int i = (int) (REG_FIRST); i < (int) (REG_LAST); ++i)
    {
        REG reg = (REG)(i);
        if (REG_StringShort(reg) == reg_string)
        {
            return reg;
        }
    }
    cerr << "Invalid register specified at command line: " << reg_string << endl;
    exit(2);
}

/*!
 * The main procedure of the tool.
 * This function is called when the application image is loaded but not yet started.
 * @param[in]   argc            total number of elements in the argv array
 * @param[in]   argv            array of command line arguments, 
 *                              including pin -t <toolname> -- ...
 */
int main(int argc, char *argv[])
{
    // Initialize PIN library. Print help message if -h(elp) is specified
    // in the command line or the command line is invalid 
    if( PIN_Init(argc,argv) )
    {
        return Usage();
    }
    
    reg_to_check = FindRegToTest();

    // Initialize the memory reference buffer
    // Use 64 bytes record, just in case
    bufId = PIN_DefineTraceBuffer(64, NUM_BUF_PAGES,
                                  BufferFull, 0);

    if(bufId == BUFFER_ID_INVALID)
    {
        cerr << "Error allocating initial buffer" << endl;
        return 1;
    }

    // add an instrumentation function
    TRACE_AddInstrumentFunction(Trace, 0);
    
    buf_key = PIN_CreateThreadDataKey(0);
    PIN_AddThreadStartFunction(ThreadStart, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}


