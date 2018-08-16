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
#ifndef _IALARM_H_
#define _IALARM_H_

#include "pin.H"
#include "control_chain.H"
#include <string.h>

namespace CONTROLLER 
{

//use this to avoid cache line false sharing on counters
struct CACHELINE_COUNTER {
    UINT64 _count;
    UINT8 _pad[56];
};

//this is the interface class for all the alarms
class IALARM
{
public:
    IALARM(UINT32 tid ,UINT64 count,BOOL need_ctxt, 
           ALARM_MANAGER* manager){
        _tid = tid;
        _target_count._count = count;
        _need_context = need_ctxt;
        _alarm_manager = manager;
        memset(_thread_count,0,sizeof(_thread_count));
        memset(_armed,0,sizeof(_armed));
        PIN_InitLock(&_lock);
    }

    //arms all threads
    VOID Arm();

    //arms only thread id tid
    VOID Arm(THREADID tid) {_armed[tid] = 1; }
    
    //disarm alarm for thread is tid and init the counter
    VOID Disarm(THREADID tid);

    //disarm alarm for thread is tid and init the counter
    VOID Disarm();

    //set the number of counts to raise the vent after
    VOID SetCount(UINT64 count) {_target_count._count = count;}
    
protected:
    
    UINT32 GetInstrumentOrder();

    //add if analysis function 
    static VOID InsertIfCall_Count(IALARM* alarm, INS ins, UINT32 ninst);
    
    //add then analysis function
    static VOID InsertThenCall_Fire(IALARM* alarm, INS ins);

    //return True if: 1. the alarm is armed 
    //                2. we are in the correct tid
    //                3. we reached the target count
    static ADDRINT PIN_FAST_ANALYSIS_CALL Count(IALARM* ialarm, UINT32 tid,
                                                  UINT32 ninst);

    //do fire 
    static VOID Fire(IALARM* ialarm, CONTEXT* ctxt, VOID * ip, UINT32 tid);

    //whether we need context
    BOOL _need_context;
    
    //the thread Id 
    UINT32 _tid;
    
    //the count that we need to reach to fire
    CACHELINE_COUNTER _target_count;
    
    //counter per thread
    CACHELINE_COUNTER _thread_count[PIN_MAX_THREADS];
    
    BOOL _armed[PIN_MAX_THREADS];

    ALARM_MANAGER* _alarm_manager;
private:
    
#if defined(TARGET_WINDOWS)
    __declspec(align(64))
#else
    __attribute__ ((aligned(64)))
#endif
    PIN_LOCK _lock;
    
    
};

} //namespace
#endif
