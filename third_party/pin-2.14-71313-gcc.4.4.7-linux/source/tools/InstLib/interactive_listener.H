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
#ifndef _INTERACTIVE_LISTENER_H_
#define _INTERACTIVE_LISTENER_H_

#include "pin.H"
#include <string>

using namespace std;
namespace CONTROLLER {

class INTERACTIVE_LISTENER{
public:
    INTERACTIVE_LISTENER(const string& file_name):
        _file_name(file_name),_signaled(FALSE){}

#if !defined(TARGET_WINDOWS)
    VOID Active();
      
    //check atomically if we had a signal
    inline UINT32 CheckClearSignal(){    
        //using inline asm since we have old compilers that do not support
        //the __sync_val_compare_and_swap function
        int value = 1;
        int new_val = 0;
          __asm__ __volatile__("lock; cmpxchgl %1,%2"
                : "=a"(value)
                : "r"(new_val), "m"(_signaled), "0"(value)
                : "memory");
          return value;
    }
#else
    VOID Active(){}
    inline UINT32 CheckClearSignal(){ return 0; }
#endif


private:
#if !defined(TARGET_WINDOWS)        
    static VOID Fini(INT32, VOID* v);
    static VOID WaitForUserSiganl(VOID* v);
    static VOID MonitorFD(THREADID tid, CONTEXT *ctxt, SYSCALL_STANDARD std, VOID *v);
    static VOID AfterForkInChild(THREADID tid, const CONTEXT* ctxt, void* v);
    
    UINT32 OpenSocket();

#endif    
    string _file_name;
    string _full_file;
    UINT32 _main_pid;
    
    //using volatile since this member accessed from several threads
    volatile UINT32 _signaled;
    UINT32 _server_sock;
};


} //namespace
#endif
