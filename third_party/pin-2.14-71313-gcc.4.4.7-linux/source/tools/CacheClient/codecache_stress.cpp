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
#include <iostream>
#include <fstream>
#include <time.h>
#include <stdlib.h>
#include "pin.H"

ofstream OutFile;
clock_t ToolStartTime;
int CodeCacheFlushTimes = 0;

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "codecache_stress.out", "specify output file name");
KNOB<int> KnobFlushTimesThreshold(KNOB_MODE_WRITEONCE, "pintool",
    "x", "50", "specify threshold for the number of times PIN flushes the code cache");

// This function is called when PIN flushes the code cache
VOID CacheFlushedCallback()
{
    CodeCacheFlushTimes++;
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    double executionTime = ((clock()-ToolStartTime)*1000L)/CLOCKS_PER_SEC;

    OutFile.precision(3);
    // Write to a file since cout and cerr maybe closed by the application
    OutFile << "Program ran for " << std::fixed << executionTime << " milli seconds" << endl;
    OutFile << "PIN code cache size is " << CODECACHE_CodeMemUsed() << " bytes" << endl;
    OutFile << "PIN code cache was flushed " << CodeCacheFlushTimes << " times" << endl;
    if (CodeCacheFlushTimes < KnobFlushTimesThreshold.Value())
    {
        OutFile << "The threshold for number of code cache flushes wasn't met" << endl;
        abort();
    }

    OutFile.close();
}
/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 CodeCacheStressUsage()
{
    cerr << "This Pintool prints statistics related to PIN's code cache such as code cache size "
            "and checks whether a specified threshold of number of code cache flushes was met" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char * argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return CodeCacheStressUsage();

    OutFile.open(KnobOutputFile.Value().c_str());


    // Register CacheFlushedCallback to be called when the PIN flushes the code cache
    CODECACHE_AddCacheFlushedFunction(CacheFlushedCallback, NULL);
    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    ToolStartTime = clock();
    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
