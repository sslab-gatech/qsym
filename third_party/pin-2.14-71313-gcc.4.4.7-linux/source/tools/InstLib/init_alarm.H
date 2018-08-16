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
#ifndef _INIT_ALARM_H_
#define _INIT_ALARM_H_

#include "pin.H"
#include <set>

namespace CONTROLLER{
class CONTROL_MANAGER;

class INIT_ALARM{
public:
    VOID Activate(CONTROL_MANAGER* manager);

private:
    CONTROL_MANAGER* _manager;
    ADDRINT _start_thread[PIN_MAX_THREADS];
    set<ADDRINT> _thread_first_ip;

    static VOID OnTrace(TRACE trace, VOID *vthis);
    static VOID ThreadStart(THREADID tid, CONTEXT *ctxt, INT32 flags, VOID *v);
    static inline ADDRINT ShouldStart(INIT_ALARM* init_alarm, THREADID tid){
        return init_alarm->_start_thread[tid];
    }
    static VOID Start(CONTEXT *ctxt, ADDRINT ip, THREADID tid, VOID *vthis);
};

} //namespace
#endif
