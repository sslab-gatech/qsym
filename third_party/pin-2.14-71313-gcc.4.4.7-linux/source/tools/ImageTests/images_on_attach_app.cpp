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
 *  This application should be used with the images_on_attach_tool tool.
 *  See documentation in the tool for the test details.
 */

#include <iostream>
#include <cstdlib>
#include <dlfcn.h>
#include <unistd.h>

using std::cout;
using std::cerr;
using std::endl;


extern "C"
{
void DoRelease(volatile bool* doRelease)
{
    // Do nothing
}
} // extern "C"


static void WaitForAttach()
{
    const unsigned int timeout = 300;
    unsigned int releaseCounter = 0;
    volatile bool released = false;
    while (!released)
    {
        if (timeout == releaseCounter)
        {
            cerr << "APP ERROR: Timeout reached and the tool did not release the application." << endl;
            exit(1);
        }
        ++releaseCounter;
        DoRelease(&released);
        sleep(1);
    }
}


static void LoadAdditionalLibraries(const char* usrlib)
{
    const void *libutil = dlopen("libutil.so.1", RTLD_LAZY);
    if(NULL == libutil)
    {
        cerr << "APP ERROR: Failed to load libutil.so.1" << endl;
        exit(1);
    }
    const void *usrlibptr = dlopen(usrlib, RTLD_LAZY);
    if(NULL == usrlibptr)
    {
        cerr << "APP ERROR: Failed to load " << usrlib << endl;
        exit(1);
    }
}


/*
 * Expected arguments:
 *
 * [1] - Shared object to load dynamically
 */
int main( int argc, char *argv[] )
{
    // Check the number of parameters.
    if (2 != argc)
    {
        cerr << "Usage: " << argv[0] << " <shared object to load>" << endl;
        return 1;
    }

    // Wait for the tool to attach to the application.
    WaitForAttach();

    // Pin is attached, now load two more shared objects.
    LoadAdditionalLibraries(argv[1]);

    // Done.
    cout << "APP: Application completed successfully." << endl;
    return 0;
}
