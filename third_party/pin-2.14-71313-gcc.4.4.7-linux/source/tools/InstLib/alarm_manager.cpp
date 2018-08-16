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

#include <sstream> 
#include "alarm_manager.H"
#include "controller_events.H"
#include "parse_control.H"
#include "alarms.H"
#include "pin.H"


using namespace CONTROLLER;
                      
map<string, ALARM_TYPE> ALARM_MANAGER::InitAlarms(){
    map<string, ALARM_TYPE> alarm_map;
    alarm_map["icount"] = ALARM_TYPE_ICOUNT;
    alarm_map["address"] = ALARM_TYPE_ADDRESS;
    alarm_map["ssc"] = ALARM_TYPE_SSC;
    alarm_map["itext"] = ALARM_TYPE_ITEXT;
    alarm_map["int3"] = ALARM_TYPE_INT3;
    alarm_map["isa_extension"] = ALARM_TYPE_ISA_EXTENSION;
    alarm_map["isa_category"] = ALARM_TYPE_ISA_CATEGORY;
    alarm_map["interactive"] = ALARM_TYPE_INTERACTIVE;
    alarm_map["enter_func"] = ALARM_TYPE_ENTER_FUNC;
    alarm_map["exit_func"] = ALARM_TYPE_EXIT_FUNC;

    return alarm_map;
}

ALARM_TYPE ALARM_MANAGER::GetAlarmType(const string& alarm_name){
    map<string, ALARM_TYPE>::iterator iter = _alarm_map.find(alarm_name);
    if (iter == _alarm_map.end()){
        ASSERT(FALSE,"Unsupported alarm: " + alarm_name);
    }
    return iter->second;
}


ALARM_MANAGER::ALARM_MANAGER(const string& control_str,
                             CONTROL_CHAIN* control_chain,
                             UINT32 id){
    _bcast = FALSE;
    _tid = ALL_THREADS;
    _count = 1;
    _raw_alarm = control_str;
    _control_chain = control_chain;
    _uniform_type = FALSE;
    _arm_next = TRUE;
    _id = id;
    _alarm_map = InitAlarms();
    
    vector<string> control_tokens;
    
    PARSER::SplitArgs(":",control_str,control_tokens);   
    if (PARSER::UniformToken(control_tokens)){
        ParseUniform(control_tokens);
        ParseCommon(control_tokens);
        
        _ialarm = GenUniform();
    }
    else{
        ParseEventId(control_tokens);
        ParseAlarm(control_tokens);
        ParseCommon(control_tokens);

        _ialarm = GenerateAlarm();
    }
}

IALARM* ALARM_MANAGER::GenUniform(){
    BOOL ctxt = _control_chain->NeedContext();
    //FIXME: add comment
    return new ALARM_ICOUNT("1",_tid,1,ctxt,this);
}


IALARM* ALARM_MANAGER::GenAddress(){

    string hex = "0x";
    BOOL ctxt = _control_chain->NeedContext();
    if (_alarm_value.compare(0, 2, hex) == 0){
        //this is a raw address
        return new ALARM_ADDRESS(_alarm_value,_tid,_count,ctxt,this);
    }

    
    if (_alarm_value.find("+",0) == string::npos){
        //this is a symbol
        return new ALARM_SYMBOL(_alarm_value,_tid,_count,ctxt,this);
    }

    else{
        vector<string> tokens;
        PARSER::SplitArgs("+",_alarm_value,tokens);
        return new ALARM_IMAGE(tokens[0],tokens[1],_tid,_count,ctxt,this);
    }

}


IALARM* ALARM_MANAGER::GenerateAlarm(){
    BOOL ctxt = _control_chain->NeedContext();
    switch (_alarm_type)
    {
    case ALARM_TYPE_ICOUNT: return new ALARM_ICOUNT(_alarm_value,_tid,_count,
                                                      ctxt,this);
    case ALARM_TYPE_ADDRESS: return GenAddress();
    case ALARM_TYPE_SSC: return new ALARM_SSC(_alarm_value,_tid,_count,
                                              ctxt,this);
    case ALARM_TYPE_ISA_EXTENSION: return new ALARM_ISA_EXTENSION(_alarm_value,
                                                _tid,_count,ctxt,this);
    case ALARM_TYPE_ISA_CATEGORY: return new ALARM_ISA_CATEGORY(_alarm_value,
                                                                _tid,_count,
                                                                ctxt,this);
    case ALARM_TYPE_ITEXT: return new ALARM_ITEXT(_alarm_value,_tid,_count,ctxt,
                                                  this);
    case ALARM_TYPE_INT3: return new ALARM_INT3(_alarm_value,_tid,_count,ctxt,
                                                this);
    case ALARM_TYPE_INTERACTIVE: return new ALARM_INTERACTIVE(_tid,ctxt,this);
    case ALARM_TYPE_ENTER_FUNC: return new ALARM_ENTER_FUNC(_alarm_value,_tid,_count,ctxt,
                                                            this);
    case ALARM_TYPE_EXIT_FUNC: return new ALARM_EXIT_FUNC(_alarm_value,_tid,_count,ctxt,
                                                          this);

    default: ASSERT(FALSE,"Unexcpected alarm");
    }
    return NULL; //pacify the compiler 
}

VOID ALARM_MANAGER::ParseUniform(vector<string>& control_tokens){
    ASSERT(control_tokens.size() > 3,"usage uniform:<period>:<length>:<count>");

    _event_name = "uniform";
    _event_type = EVENT_START;
    _uniform_type = TRUE;
    _arm_next = FALSE;
    
    _uniform_period = PARSER::StringToUint64(control_tokens[1]);
    _uniform_length = PARSER::StringToUint64(control_tokens[2]);
    _uniform_count  = PARSER::StringToUint64(control_tokens[3]);
    control_tokens.erase(control_tokens.begin(),control_tokens.begin()+4);

    ASSERT(_uniform_period >= _uniform_length,"uniform period must be "
                                              "larger than uniform length");

    _control_chain->SetUniformAlarm(this);
}

VOID ALARM_MANAGER::ParseEventId(vector<string>& control_tokens){
    ASSERT(control_tokens.size() > 0,"Usage: no event");
    
    _event_name = control_tokens[0];
    _event_type = _control_chain->EventStringToType(_event_name);
    control_tokens.erase(control_tokens.begin());
}

VOID ALARM_MANAGER::ParseAlarm(vector<string>& control_tokens){
    
    
    _alarm_name = control_tokens[0];
    _alarm_type = GetAlarmType(_alarm_name);
    
    if (_alarm_type == ALARM_TYPE_INTERACTIVE){
        control_tokens.erase(control_tokens.begin());
        return;
    }
    ASSERT(control_tokens.size() > 1,"Usage Error: Alarm with no value");
    control_tokens.erase(control_tokens.begin());

    _alarm_value = control_tokens[0];
    control_tokens.erase(control_tokens.begin());

}


VOID ALARM_MANAGER::ParseCommon(vector<string>& control_tokens){
    for (UINT32 i = 0; i < control_tokens.size(); i++){
        string token = control_tokens[i];
        PARSER::ParseTIDToken(token, &_tid);
        PARSER::ParseBcastToken(token, &_bcast);
        PARSER::ParseCountToken(token, &_count);
    }
}

VOID ALARM_MANAGER::SetNextUniformEvent(THREADID tid){
    if (_event_type == EVENT_START){
        _event_type = EVENT_STOP;
        _ialarm->SetCount(_uniform_length);
        _ialarm->Arm();
        return;
    }

    if (_event_type == EVENT_STOP){
        //completed one uniform cycle
        _uniform_count--;
        
        if (_uniform_count == 0){
            //completed all uniform cycles, we can now arm the next alarm 
            //in the chain(if exists)
            _arm_next = TRUE;
        }
        else{
            _event_type = EVENT_START;
            //setting the icount for the region until we get to "start"
            _ialarm->SetCount(_uniform_period - _uniform_length);
            _ialarm->Arm();
        }
        return;
    }
}


VOID ALARM_MANAGER::Fire(CONTEXT* ctx, VOID * ip, THREADID tid){
    Disarm(tid);
    EVENT_TYPE ev = _event_type;
    BOOL bcast = _bcast;
    
    //saving the current event type since SetNextEvent will modify it.
    //must do SetNextEvent before Fire since its side effects are needed in 
    //the fire function
    if (_uniform_type && !IsUniformDone()){
        //for uniform we reset all the counters
        Disarm();
        bcast = TRUE;
        SetNextUniformEvent(tid);
    }
    
    _control_chain->Fire(ev,ctx,ip,tid,bcast, _id);
}

BOOL ALARM_MANAGER::HasStartEvent(){
    return (_event_type == EVENT_START);
}

VOID ALARM_MANAGER::Print(){
    cerr << "EVENT: " << _event_name <<endl;
    cerr << "ALARM: " << _alarm_name <<endl;
    cerr << "VALUE: " << _alarm_value <<endl;
    cerr << "TID: " << _tid <<endl;
    cerr << "BCAST:" << _bcast <<endl;
    cerr << "COUNT:" << _count <<endl;
}

BOOL ALARM_MANAGER::IsUniformDone(){
    return _uniform_count == 0;
}

VOID ALARM_MANAGER::Disarm(){
    _ialarm->Disarm();
}

VOID ALARM_MANAGER::Disarm(THREADID tid){
    if (_bcast){
        _ialarm->Disarm();
    }
    else{
        _ialarm->Disarm(tid);
    }
}

VOID ALARM_MANAGER::Activate(){
    if (_tid == ALL_THREADS){
        ArmAll();
    }
    else{
        ArmTID(_tid);
    }
}
