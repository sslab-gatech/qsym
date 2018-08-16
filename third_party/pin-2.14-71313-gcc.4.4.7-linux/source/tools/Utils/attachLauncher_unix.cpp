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
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::string;
using std::stringstream;


static void ParseArguments(const int argc, const char* argv[], vector<string>& pinCmd, vector<string>& appCmd)
{
    if (argc < 4)
    {
        cerr << "LAUNCHER ERROR: Too few arguments to the launcher. Expected at least the following:" << endl
             << "  <launcher> <pin> -- <app>" << endl;
        exit(1);
    }
    const string pin(argv[1]);
    if (string::npos == pin.find("pin"))
    {
        cerr << "LAUNCHER ERROR: Expected to find Pin as the first argument but found '" << pin << "' instead." << endl;
        exit(1);
    }
    pinCmd.push_back(pin);
    unsigned int i = 2;
    for (; i < argc; ++i)
    {
        const string token(argv[i]);
        if ("--" == token) break;
        pinCmd.push_back(token);
    }
    if (argc == i)
    {
        cerr << "LAUNCHER ERROR: Could not find the application delimiter '--'." << endl;
        exit(1);
    }
    for (++i; i < argc; ++i)
    {
        const string token(argv[i]);
        appCmd.push_back(token);
    }
}


static pid_t LaunchApp(const vector<string>& appCmd)
{
    // Prepare the argument list.
    const unsigned int appArgc = appCmd.size();
    char** appArgv = new char*[appArgc + 1]; // additional slot for the NULL terminator
    for (unsigned int i = 0; i < appArgc; ++i)
    {
        appArgv[i] = strdup(appCmd[i].c_str());
    }
    appArgv[appArgc] = NULL; // add the NULL terminator

    // Launch the application.
    const pid_t child = fork();
    if (-1 == child)
    {
        perror("LAUNCHER ERROR: Failed to fork the application process");
        exit(1);
    }
    else if (0 == child)
    {
        // In the child process.
        cout << endl << "LAUNCHER: Running the application with pid [" << getpid() << "]:" << endl << appArgv[0];
        for (unsigned int i = 1; NULL != appArgv[i]; ++i)
        {
            cout << " " << appArgv[i];
        }
        cout << endl;
        const int res = execvp(appArgv[0], appArgv); // does not return on success
        perror("LAUNCHER ERROR: In the child process. Failed to execute the application");
        exit(1);
    }

    // In the parent process.
    return child;
}


static pid_t LaunchPin(const vector<string>& pinCmd, const pid_t appPid)
{
    // Prepare the argument list.
    const unsigned int pinArgc = pinCmd.size();
    char** pinArgv = new char*[pinArgc + 3]; // two additional slots: "-pid <appPid>" and one for the NULL terminator
    pinArgv[0] = strdup(pinCmd[0].c_str());

    // Add the attach arguments.
    pinArgv[1] = "-pid";
    stringstream appPidStrm; // prepare the application's pid as a string
    appPidStrm << appPid;
    pinArgv[2] = strdup(appPidStrm.str().c_str());

    // Add the rest of the arguments for Pin (if they exist).
    for (unsigned int origArgs = 1, newArgs = 3; origArgs < pinArgc; ++origArgs, ++newArgs)
    {
        pinArgv[newArgs] = strdup(pinCmd[origArgs].c_str());
    }
    pinArgv[pinArgc + 2] = NULL; // add the NULL terminator

    // Launch Pin.
    const pid_t child = fork();
    if (-1 == child)
    {
        perror("LAUNCHER ERROR: Failed to fork the Pin process");
        exit(1);
    }
    else if (0 == child)
    {
        // In the child process.
        cout << endl << "LAUNCHER: Running Pin:" << endl << pinArgv[0];
        for (unsigned int i = 1; NULL != pinArgv[i]; ++i)
        {
            cout << " " << pinArgv[i];
        }
        cout << endl << endl;
        const int res = execvp(pinArgv[0], pinArgv); // does not return on success
        perror("LAUNCHER ERROR: In the child process. Failed to execute Pin");
        exit(1);
    }

    // In the parent process.
    return child;
}


static void WaitForPin(const pid_t pinPid, const pid_t appPid)
{
    int pinStatus = 0;
    if (pinPid != waitpid(pinPid, &pinStatus, 0))
    {
        perror("LAUNCHER ERROR: Encountered an error while waiting for Pin to exit");
        kill(pinPid, SIGKILL);
        kill(appPid, SIGKILL);
        exit(1);
    }
    if (!WIFEXITED(pinStatus))
    {
        cerr << "LAUNCHER ERROR: The Pin process sent a notification to the launcher without exiting." << endl;
        kill(pinPid, SIGKILL);
        kill(appPid, SIGKILL);
        exit(1);
    }
    else
    {
        int pinCode = WEXITSTATUS(pinStatus);
        if (0 != pinCode)
        {
            cerr << "LAUNCHER ERROR: Pin exited with an abnormal return value: " << pinCode << endl;
            kill(appPid, SIGKILL);
            exit(pinCode);
        }
    }
}


static void WaitForApp(const pid_t appPid)
{
    int appStatus = 0;
    if (appPid != waitpid(appPid, &appStatus, 0))
    {
        perror("LAUNCHER ERROR: Encountered an error while waiting for the application to exit");
        kill(appPid, SIGKILL);
        exit(1);
    }
    if (!WIFEXITED(appStatus))
    {
        cerr << "LAUNCHER ERROR: The application sent a notification to the launcher without exiting." << endl;
        kill(appPid, SIGKILL);
        exit(1);
    }
    else
    {
        int appCode = WEXITSTATUS(appStatus);
        if (0 != appCode)
        {
            cerr << "LAUNCHER ERROR: The application exited with an abnormal return value: " << appCode << endl;
            exit(appCode);
        }
    }
}


int main(const int argc, const char* argv[])
{
    // Parse the given command line and break it down to Pin and application command lines.
    vector<string> pinCmd;
    vector<string> appCmd;
    ParseArguments(argc, argv, pinCmd, appCmd);

    // Launch the application.
    const pid_t appPid = LaunchApp(appCmd);

    // Launch Pin and attach to the application.
    const pid_t pinPid = LaunchPin(pinCmd, appPid);

    // Wait for Pin to return.
    WaitForPin(pinPid, appPid);

    // Wait for the application to return.
    WaitForApp(appPid);

    // Done.
    return 0;
}
