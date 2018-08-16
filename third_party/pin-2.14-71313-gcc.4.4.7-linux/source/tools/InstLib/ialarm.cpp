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

#include "ialarm.H"
#include "alarm_manager.H"
#include "parse_control.H"
#include "control_chain.H"
#include <iostream> 


using namespace CONTROLLER;


VOID IALARM::InsertIfCall_Count(IALARM* alarm, INS ins, UINT32 ninst){
    INS_InsertIfCall(ins, IPOINT_BEFORE,
        AFUNPTR(Count),
        IARG_FAST_ANALYSIS_CALL,
        IARG_CALL_ORDER, alarm->GetInstrumentOrder(),
        IARG_ADDRINT, alarm,
        IARG_THREAD_ID,
        IARG_UINT32, ninst,
        IARG_END);
}

VOID IALARM::InsertThenCall_Fire(IALARM* alarm, INS ins){
    if (alarm->_need_context){
        INS_InsertThenCall(ins, IPOINT_BEFORE,
            AFUNPTR(Fire),
            IARG_CALL_ORDER, alarm->GetInstrumentOrder(),
            IARG_ADDRINT, alarm,
            IARG_CONTEXT, 
            IARG_INST_PTR,
            IARG_THREAD_ID,
            IARG_END);
    }
    else{
        INS_InsertThenCall(ins, IPOINT_BEFORE,
            AFUNPTR(Fire),
            IARG_CALL_ORDER, alarm->GetInstrumentOrder(),
            IARG_ADDRINT, alarm,
            IARG_ADDRINT, static_cast<ADDRINT>(0), // pass a null as context, 
            IARG_INST_PTR,
            IARG_THREAD_ID,
            IARG_END);
    }
}

ADDRINT PIN_FAST_ANALYSIS_CALL IALARM::Count(IALARM* ialarm, 
                                               UINT32 tid,
                                               UINT32 ninst){
    UINT32 armed = ialarm->_armed[tid];
    UINT32 correct_tid = (ialarm->_tid == tid) | (ialarm->_tid == ALL_THREADS);

    UINT32 should_count = armed & correct_tid;

    //if we are not in the correct thread
    ialarm->_thread_count[tid]._count += ninst*(should_count);

    return ialarm->_thread_count[tid]._count >= ialarm->_target_count._count;

}

//we want to generate the context only when we really need it.
//that is way most of the code is in the If instrumentation.
//even if the If instrumentation is be not inlined.
VOID IALARM::Fire(IALARM* ialarm, CONTEXT* ctxt, VOID * ip, UINT32 tid){
    ialarm->_alarm_manager->Fire(ctxt, ip, tid);
}

VOID IALARM::Arm(){
    PIN_GetLock(&_lock,0);
    memset(_armed,1,sizeof(_armed));
    PIN_ReleaseLock(&_lock);
}

VOID IALARM::Disarm(THREADID tid){
    _armed[tid] = 0;
    _thread_count[tid]._count = 0;
}

VOID IALARM::Disarm(){
    PIN_GetLock(&_lock,0);
    memset(_armed,0,sizeof(_armed));
    memset(_thread_count,0,sizeof(_thread_count));
    PIN_ReleaseLock(&_lock);
}

UINT32 IALARM::GetInstrumentOrder(){
    return _alarm_manager->GetInsOrder();
}
