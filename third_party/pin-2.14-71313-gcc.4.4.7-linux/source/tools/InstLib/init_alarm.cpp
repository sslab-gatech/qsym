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


#include "init_alarm.H"
#include "control_manager.H"


using namespace CONTROLLER;

VOID INIT_ALARM::Activate(CONTROL_MANAGER* manager){

    _manager = manager;
    memset(_start_thread,0,sizeof(_start_thread));

    PIN_CALLBACK thread_start = PIN_AddThreadStartFunction(ThreadStart, this);
    // other tools working with the controller might do some initialization in their 
    // thread start callback.
    // need to make sure we are been call AFTER all thread start callbacks were called.
    CALLBACK_SetExecutionPriority(thread_start, CALL_ORDER_LAST);
    TRACE_AddInstrumentFunction(OnTrace, this);
}

VOID INIT_ALARM::ThreadStart(THREADID tid, CONTEXT *ctxt, INT32 flags, VOID *v){
    INIT_ALARM *me = static_cast<INIT_ALARM*>(v);
    me->_start_thread[tid] = TRUE;

    ADDRINT first_ip = PIN_GetContextReg(ctxt, REG_INST_PTR);
    me->_thread_first_ip.insert(first_ip);
    // this IP might be already instrumented, so we need to reset 
    // the instrumentations on it's BB.
    CODECACHE_InvalidateRange(first_ip, first_ip+15);
}

VOID INIT_ALARM::OnTrace(TRACE trace, VOID *vthis){
    INIT_ALARM *me = static_cast<INIT_ALARM*>(vthis);

    INS ins = BBL_InsHead(TRACE_BblHead(trace));
    ADDRINT first_ip = INS_Address(ins); 
    
    if (me->_thread_first_ip.find(first_ip) == me->_thread_first_ip.end()){
        // the first instruction of the TARCE is not the first instruction of
        // any new created thread 
        return;
    }
    INS_InsertIfCall(
        ins, IPOINT_BEFORE, AFUNPTR(ShouldStart),
        IARG_CALL_ORDER, me->_manager->GetInsOrder(),
        IARG_ADDRINT, me,
        IARG_THREAD_ID,
        IARG_END);

    if (me->_manager->PassContext())
    {
        INS_InsertThenCall(
            ins, IPOINT_BEFORE, AFUNPTR(Start),
            IARG_CALL_ORDER, me->_manager->GetInsOrder(),
            IARG_CONTEXT, 
            IARG_INST_PTR,
            IARG_THREAD_ID, 
            IARG_ADDRINT, me, 
            IARG_END);
    }
    else
    {
        INS_InsertThenCall(
            ins, IPOINT_BEFORE, AFUNPTR(Start),
            IARG_CALL_ORDER, me->_manager->GetInsOrder(),
            IARG_ADDRINT, static_cast<ADDRINT>(0),
            IARG_INST_PTR,
            IARG_THREAD_ID, 
            IARG_ADDRINT, me,
            IARG_END);
    }
}

VOID INIT_ALARM::Start(CONTEXT *ctxt, ADDRINT ip, THREADID tid, VOID *vthis){
    INIT_ALARM *me = static_cast<INIT_ALARM*>(vthis);
    
    me->_start_thread[tid] = FALSE;
    me->_manager->Fire(EVENT_START,ctxt,Addrint2VoidStar(ip),tid,TRUE);
}

