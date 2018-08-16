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
#include <vector>
#include <algorithm>
#include <string>

void initSkipList(std::vector<std::string>& skipList)
{
    skipList.push_back("PPID");
    skipList.push_back("PIN_");
    skipList.push_back("_=");
    skipList.push_back("SSH_CLIENT");
    skipList.push_back("SSH_CONNECTION");
    skipList.push_back("RANDOM");
    skipList.push_back("MFLAGS");
}

int main(int argc, char **argv, char **envp)
{
    std::vector<std::string> skipList;
    initSkipList(skipList);

    std::vector<std::string> e;

    for (char **currVar = envp; *currVar != NULL; ++currVar)
    {
        bool skipCurrVar = false;
        std::string currStr(*currVar);
        for (std::vector<std::string>::iterator it=skipList.begin(); it != skipList.end(); ++it)
        {
            if (currStr.compare(0, (*it).size(), *it) == 0)
            {
                skipCurrVar = true;
                break;
            }
        }

        if (skipCurrVar) continue;
        e.push_back(currStr);
    }
    // Pin might change the order of the environment variables, so we want to
    // print them sorted, so print order will be the same for the same env.
    // 
    std::sort(e.begin(), e.end());

    for (std::vector<std::string>::iterator it=e.begin(); it != e.end(); ++it)
    {
        std::cout << *it << std::endl;
    }

    return 0;
}
