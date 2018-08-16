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

#include "control_chain.H"
#include "parse_control.H"
#include "alarm_manager.H"
#include "controller_events.H"

using namespace std;
using namespace CONTROLLER;

UINT32 CONTROL_CHAIN::global_id = 0;

CONTROL_CHAIN::CONTROL_CHAIN(CONTROL_MANAGER* control_mngr){
    _control_mngr = control_mngr;
    _name = "";
    _wait_for_id = NO_WAIT;
    _repeat_token = 1;
    _id = global_id;
    global_id++;

    memset(_repeat,0,sizeof(_repeat));
}


VOID CONTROL_CHAIN::Fire(EVENT_TYPE eventID, CONTEXT* ctx, VOID * ip, 
    THREADID tid, BOOL bcast, UINT32 alarm_id)
{
    //roll the event to the tool only if it is not a precondition 
    if (eventID != EVENT_PRECOND){
        _control_mngr->Fire(eventID, ctx, ip, tid, bcast);
    }
    if (_alarms[alarm_id]->ArmNext()){
        ArmNextAlarm(alarm_id, tid, bcast);
    }
}

VOID CONTROL_CHAIN::Activate(){
    //activate the first alarm in the chain only if the chain does not "wait"
    if (_wait_for_id == NO_WAIT){
        _alarms[0]->Activate();
    }
}


VOID CONTROL_CHAIN::ArmChain(UINT32 tid){
    _alarms[0]->ArmTID(tid);
}

VOID CONTROL_CHAIN::Arm(UINT32 tid, BOOL bcast, UINT32 alarm_id){
    if (bcast){
        //if we are in broadcast arm all threads in the next alarm
        _alarms[alarm_id]->ArmAll();
    }
    else{
        _alarms[alarm_id]->ArmTID(tid);
    }
}


VOID CONTROL_CHAIN::Parse(const string& chain_str)
{
    vector<string> control_str;
    
    PARSER::SplitArgs(",",chain_str,control_str);
    for (UINT32 i=0; i<control_str.size(); i++){
        if (PARSER::ConfigToken(control_str[i])){
            //parse all the config tokes(repeat,name,waitfor)
            PARSER::ParseConfigTokens(control_str[i],this);
        }
        else {
            //generate the alarm
            ALARM_MANAGER* alarm_mngr = new ALARM_MANAGER(control_str[i],
                                                          this,i);
            _alarms.push_back(alarm_mngr);
        }
    }
}

BOOL CONTROL_CHAIN::NeedContext(){
    return _control_mngr->PassContext();
}

BOOL CONTROL_CHAIN::NeedToRepeat(UINT32 tid){
    if (_repeat[tid] < _repeat_token || _repeat_token == REPEAT_INDEFINITELY){
            return TRUE;
    }
    return FALSE;
}

VOID CONTROL_CHAIN::ArmNextAlarm(UINT32 alarm_id, UINT32 tid, BOOL bcast){
    UINT32 last_alarm_id = _alarms.size() - 1;
    if (alarm_id < last_alarm_id){
        //we still have alarm after this one
        Arm(tid,bcast,alarm_id+1);
    }
    else{
        //we are in the last alarm check the repeat
        _repeat[tid]++;
        if (NeedToRepeat(tid)){
            Arm(tid,bcast,0);
        }
        else{
            ArmWaitingChains(tid);
        }
    }
}

VOID CONTROL_CHAIN::SetWaitFor(const string& chain_name){
    UINT32 id = _control_mngr->GetChainId(chain_name);
    SetWaitFor(id);
}

VOID CONTROL_CHAIN::SetWaitFor(UINT32 chain_id){
    CONTROL_CHAIN* chain = _control_mngr->ChainById(chain_id);
    if (chain == NULL){
        stringstream s;
        s << "chain id " << chain_id << " does not exists";
        ASSERT(FALSE, s.str());
    }
    _wait_for_id = chain_id;
    chain->AddWaitingChain(this);
}

VOID CONTROL_CHAIN::AddWaitingChain(CONTROL_CHAIN* chain){
    _waiting_chains.push_back(chain);
}

VOID CONTROL_CHAIN::ArmWaitingChains(UINT32 tid){
    list<CONTROL_CHAIN*>::iterator iter = _waiting_chains.begin();
    for(; iter != _waiting_chains.end(); iter++){
        CONTROL_CHAIN* chain = *iter;
        chain->ArmChain(tid);
    }
}

//print debug massages - only when the debug knob is used
VOID CONTROL_CHAIN::DebugPrint(){
    for (UINT32 i = 0; i < _alarms.size(); i++){
        _alarms[i]->Print();
    }
    cerr << "REPEAT: " << _repeat_token << endl;
    cerr << "NAME: " << _name << endl;
    cerr << "WAIT FOR: " << _wait_for_id << endl;
}

BOOL CONTROL_CHAIN::HasStartEvent(){
    for (UINT32 i = 0; i < _alarms.size(); i++){
        if (_alarms[i]->HasStartEvent()){
            return TRUE;
        }
    }
    return FALSE;
}

VOID CONTROL_CHAIN::SetUniformAlarm(ALARM_MANAGER* uniform_alarm){
    if (_control_mngr->_uniform_alarm){
        ASSERT(FALSE,"Only one uniform control is allowed");
    }
    _control_mngr->_uniform_alarm = uniform_alarm;
}

EVENT_TYPE CONTROL_CHAIN::EventStringToType(const string& event_name){
    return _control_mngr->EventStringToType(event_name);
}
