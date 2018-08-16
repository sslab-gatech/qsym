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
 * NOTE: Due to the structure of tests that use this tool, its output file is opened in append mode.
 * If the test is run more than once without removing the output file for the tool, the output will
 * be concatenated with the output from the previous run. To prevent it, always delete this tool's
 * output file before running.
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include "arglist.h"

/* ===================================================================== */
/* Command line Switches */
/* ===================================================================== */

//Pin command line
KNOB<string> KnobPin(KNOB_MODE_WRITEONCE, "pintool", "pin", "", "pin full path");

//Parent configuration - Application name
KNOB<string> KnobApplication(KNOB_MODE_WRITEONCE, "pintool", "app", "", "application name");

KNOB<BOOL> KnobToolProbeMode(KNOB_MODE_WRITEONCE, "pintool", "probe", "0",
        "invoke tool in probe mode");

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "unix_parent_tool.out",
        "specify output file name");

ofstream OutFile;

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cout << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/*
 * FollowChild(CHILD_PROCESS childProcess, VOID * userData) - child process configuration
 * Sets pin command line for a child that should be created
 */
BOOL FollowChild(CHILD_PROCESS childProcess, VOID * userData)
{
    INT appArgc;
    CHAR const * const * appArgv;

    CHILD_PROCESS_GetCommandLine(childProcess, &appArgc, &appArgv);
    ARGUMENTS_LIST appCmd(appArgc, appArgv);
    string childApp(appArgv[0]);

    if (KnobApplication.Value() == childApp)
    {
        // Change pin command line if defined, otherwise just follow child
        if (!KnobPin.Value().empty())
        {
            ARGUMENTS_LIST newPinCmd;
            newPinCmd.Add(KnobPin.Value());
            newPinCmd.Add("--");
            newPinCmd.Add(appCmd.String());

            CHILD_PROCESS_SetPinCommandLine(childProcess, newPinCmd.Argc(), newPinCmd.Argv());
            OutFile << "Process to execute: " << newPinCmd.String()  << endl;
        }
        else
        {
            OutFile << "Pin command line remains unchanged" << endl;
            OutFile << "Application to execute: " << appCmd.String() << endl;
        }
        return TRUE;
    }
    OutFile << "knob val " << KnobApplication.Value() << "app " << childApp << endl;
    OutFile << "Do not run Pin under the child process" << endl;
    return FALSE;
}

/* ===================================================================== */
VOID Fini(INT32 code, VOID *v)
{
    OutFile << "In unix_parent_tool PinTool" << endl;
    OutFile.close();
}

typedef VOID (*EXITFUNCPTR)(INT code);
EXITFUNCPTR origExit;

VOID ExitInProbeMode(INT code)
{
    Fini(code, 0);
    (*origExit)(code);
}

/* ===================================================================== */

VOID ImageLoad(IMG img, VOID *v)
{
    RTN exitRtn = RTN_FindByName(img, "_exit");
    if (RTN_Valid(exitRtn) && RTN_IsSafeForProbedReplacement(exitRtn))
    {
        origExit = (EXITFUNCPTR) RTN_ReplaceProbed(exitRtn, AFUNPTR(ExitInProbeMode));
    }
    else
    {
        exitRtn = RTN_FindByName(img, "exit");    
        if (RTN_Valid(exitRtn) && RTN_IsSafeForProbedReplacement(exitRtn))
        {
            origExit = (EXITFUNCPTR) RTN_ReplaceProbed(exitRtn, AFUNPTR(ExitInProbeMode));
        }
    }
}
/* ===================================================================== */

int main(INT32 argc, CHAR **argv)
{
    if (PIN_Init(argc, argv)) return Usage();

    // Can't just open for writing because the child process' Pintool may overwrite
    // the parent process' Pintool file (when the -o parameter doesn't change).
    // Opening in append mode instead.
    OutFile.open(KnobOutputFile.Value().c_str(), ofstream::app);

    PIN_AddFollowChildProcessFunction(FollowChild, 0);

    // Never returns
    if (KnobToolProbeMode)
    {
        IMG_AddInstrumentFunction(ImageLoad, 0);
        PIN_StartProgramProbed();
    }
    else
    {
        PIN_AddFiniFunction(Fini, 0);
        PIN_StartProgram();
    }
    return 0;
}

