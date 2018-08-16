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
#include "pin.H"

using std::cout;
using std::cerr;
using std::endl;
using std::flush;


typedef void (*AppFunSig)(unsigned int, pid_t);
AppFunSig OrigThreadInit = NULL;
AppFunSig OrigThreadFini = NULL;


static RTN GetProbableRtn(IMG img, const char* name)
{
    RTN rtn = RTN_FindByName(img, name);
    if (!RTN_Valid(rtn))
    {
        cerr << "TOOL ERROR: Unable to find the function \"" << name << "\"" << endl;
        PIN_ExitProcess(101);
    }
    if (!RTN_IsSafeForProbedInsertion(rtn))
    {
        cerr << "TOOL ERROR: The function \"" << name << "\" is not safe for probe insertion" << endl;
        PIN_ExitProcess(102);
    }
    return rtn;
}


static RTN GetReplacableRtn(IMG img, const char* name)
{
    RTN rtn = RTN_FindByName(img, name);
    if (!RTN_Valid(rtn))
    {
        cerr << "TOOL ERROR: Unable to find the function \"" << name << "\"" << endl;
        PIN_ExitProcess(103);
    }
    if (!RTN_IsSafeForProbedReplacement(rtn))
    {
        cerr << "TOOL ERROR: The function \"" << name << "\" is not safe for replacement" << endl;
        PIN_ExitProcess(104);
    }
    return rtn;
}


static void OnSecondaryThreadInit(unsigned int threadNum, pid_t tid)
{
    PIN_LockClient();
    if (NULL == OrigThreadInit)
    {
        cerr << "TOOL ERROR: Attempting to call SecondaryThreadInit but it is a NULL pointer" << endl;
        PIN_ExitProcess(105);
    }
    OrigThreadInit(threadNum, tid);
    PIN_UnlockClient();
}


static void OnSecondaryThreadFini(unsigned int threadNum, pid_t tid)
{
    PIN_LockClient();
    if (NULL == OrigThreadFini)
    {
        cerr << "TOOL ERROR: Attempting to call SecondaryThreadFini but it is a NULL pointer" << endl;
        PIN_ExitProcess(106);
    }
    OrigThreadFini(threadNum, tid);
    PIN_UnlockClient();
}


static void OnSecondaryThreadWork()
{
    PIN_LockClient();
    cout << "TOOL: (" << PIN_GetTid() << ") executing OnSecondaryThreadWork" << endl << flush;
    sched_yield();
    PIN_UnlockClient();
}


static void OnReleaseThreads(ADDRINT doRelease)
{
    PIN_LockClient();
    *((bool*)doRelease) = true;
    cout << "TOOL: Released the threads, now waiting a few seconds for them to reach the lock." << endl;
    PIN_Sleep(5*1000);
    PIN_UnlockClient();
}


static VOID Image(IMG img, VOID* v)
{
    if (!IMG_IsMainExecutable(img)) return;

    RTN secondaryThreadInitRtn = GetReplacableRtn(img, "SecondaryThreadInit");
    OrigThreadInit = (AppFunSig)RTN_ReplaceProbed(secondaryThreadInitRtn, AFUNPTR(OnSecondaryThreadInit));

    RTN secondaryThreadFiniRtn = GetReplacableRtn(img, "SecondaryThreadFini");
    OrigThreadFini = (AppFunSig)RTN_ReplaceProbed(secondaryThreadFiniRtn, AFUNPTR(OnSecondaryThreadFini));

    RTN secondaryThreadWorkRtn = GetProbableRtn(img, "SecondaryThreadWork");
    RTN_InsertCallProbed(secondaryThreadWorkRtn, IPOINT_BEFORE, AFUNPTR(OnSecondaryThreadWork), IARG_END);

    RTN releaseThreadsRtn = GetProbableRtn(img, "ReleaseThreads");
    RTN_InsertCallProbed(releaseThreadsRtn, IPOINT_BEFORE, AFUNPTR(OnReleaseThreads),
                         IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
}


int main(int argc, char *argv[])
{
    PIN_Init(argc,argv);
    PIN_InitSymbols();

    IMG_AddInstrumentFunction(Image, 0);

    PIN_StartProgram(); // never returns
    return 0;
}
