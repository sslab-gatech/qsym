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

/*!@file
*  This is a test to validate right behaviour of
*   calling CODECACHE_InvalidateRange from an analysis routine."
*   test flow:
*           instrument and run foo
*           cache out foo from code cache
*           instrument and run foo
*           (Fini) check that foo was instrumented twice
*/

#include<iostream>
#include<fstream>
#include<cassert>
#include"pin.H"

#ifdef TARGET_MAC
#define FOO "_foo"
#define BAZ "_baz"
#else
#define FOO "foo"
#define BAZ "baz"
#endif

/*================================================================== */
//Global variables
/*================================================================== */

UINT64 fooInstrumentCount = 0;        //number of times the first instruction of foo is instrumented
ADDRINT fooAddress = 0;

std::ostream* out = &cerr;

/*===================================================================== */
//Command line switches
/*===================================================================== */
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,  "pintool",
   "o", "", "specify file name for invalidate_cache_analysis output");

/*===================================================================== */
//Utilities
/*===================================================================== */

/*!
*  Print out help message.
*/
INT32 invalidateCacheAnalysisUsage()
{
   cerr << "This tool counts the number of times " << FOO << "'s first line instrumentation is called." << endl;
   cerr << "It assumes the application uses CODECACHE_InvalidateRange (instrumentes on " << BAZ << ")";
   cerr << " to unload the instrumented code so that foo intrumentation will be called twice." << endl << endl;
   cerr << KNOB_BASE::StringKnobSummary() << endl;

   return -1;
}

/*===================================================================== */
//Analysis routines
/*===================================================================== */

/*!
*Make "foo" function to be cashed out from the code cash
*/
VOID bazAnalysis() {
   CODECACHE_InvalidateRange(fooAddress, fooAddress+1);
}

/*===================================================================== */
//Instrumentation callbacks
/*===================================================================== */

/*!Insert call to "baz" function so it will cache out "foo" function from code cash
* @param[in]   rtn    baz routine as found by "Image" function to be instrumented
*/
VOID bazInstrumantation(RTN rtn) {
   RTN_Open(rtn);
   RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)bazAnalysis, IARG_END);
   RTN_Close(rtn);
}

/*!Add instrumentation to the first instruction of foo, so that instumatation will be re-called
*   after every time we cache out the intrumented function
*
*   @param[in]   ins    An instruction - we expect to see the first instruction of "foo" twice
*/
VOID Instruction(INS ins, VOID* v) {
   ADDRINT insAddress = INS_Address(ins);
   if (insAddress == fooAddress) {
       fooInstrumentCount++;
   }
}


VOID Image(IMG img, VOID* v){
   //  Find the foo() function.
   RTN rtn = RTN_FindByName(img, FOO);

   if (RTN_Valid(rtn))
   {
       fooAddress = RTN_Address(rtn);
   }

   //  Find the baz() function.
   rtn = RTN_FindByName(img, BAZ);
   if (RTN_Valid(rtn))
   {
       bazInstrumantation(rtn);
   }
}



/*!
* Print out analysis results.
* This function is called when the application exits.
* @param[in]   code            exit code of the application
* @param[in]   v               value specified by the tool in the
*                              PIN_AddFiniFunction function call
*/
VOID Fini(INT32 code, VOID *v)
{
   *out << "fooInstrumentCount : " << fooInstrumentCount << " (expecting 2)"<< endl;
   assert(fooInstrumentCount == 2);

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
       return invalidateCacheAnalysisUsage();
   }
   PIN_InitSymbols();

   string fileName = KnobOutputFile.Value();

   if (!fileName.empty()) { out = new std::ofstream(fileName.c_str());}

   // Register function to be called to instrument traces
   IMG_AddInstrumentFunction(Image, 0);

   INS_AddInstrumentFunction(Instruction, 0);

   // Register function to be called when the application exits
   PIN_AddFiniFunction(Fini, 0);

   if (!KnobOutputFile.Value().empty())
   {
       cout <<  "===============================================" << endl;
       cout << "See file " << KnobOutputFile.Value() << " for analysis results" << endl;
       cout <<  "===============================================" << endl;
   }

   // Start the program, never returns
   PIN_StartProgram();

   return 0;
}

/*===================================================================== */
/*eof */
/*===================================================================== */
