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
#include "controller_events.H"

using namespace CONTROLLER;

CONTROLLER_EVENTS::CONTROLLER_EVENTS(){
    _events["invalid"] = EVENT_INVALID;
    _events["precond"] = EVENT_PRECOND;
    _events["start"] = EVENT_START;
    _events["stop"] = EVENT_STOP;
    _events["threadid"] = EVENT_THREADID;
    _events["warmup-start"] = EVENT_WARMUP_START;
    _events["warmup-stop"] = EVENT_WARMUP_STOP;
    _events["prolog-start"] = EVENT_PROLOG_START;
    _events["prolog-stop"] = EVENT_PROLOG_STOP;
    _events["epilog-start"] = EVENT_EPILOG_START;
    _events["epilog-stop"] = EVENT_EPILOG_STOP;
    _events["stats-emit"] = EVENT_STATS_EMIT;
    _events["stats-reset"] = EVENT_STATS_RESET;
}

EVENT_TYPE CONTROLLER_EVENTS::AddEvent(const string& event_name){
    map<string,EVENT_TYPE>::iterator iter = _events.find(event_name);
    if (iter != _events.end()){
        ASSERT(FALSE,"event: " + event_name + " already exists");
    }
    
    ASSERT(_events.size() < _max_user_ev, "not enough events");
    EVENT_TYPE ev = static_cast<EVENT_TYPE>(_events.size());
    _events[event_name] = ev;
    return ev;
 
}

string CONTROLLER_EVENTS::IDToString(EVENT_TYPE id){
    map<string,EVENT_TYPE>::iterator iter = _events.begin();
    for (;iter != _events.end(); iter++){
        if (iter->second == id){
            return iter->first;
        }
    }
    
    return "invalid";
    
}

EVENT_TYPE CONTROLLER_EVENTS::EventStringToType(const string& event_name){
    map<string,EVENT_TYPE>::iterator iter = _events.find(event_name);
    if (iter == _events.end()){
        ASSERT(FALSE,"Unsupported event" + event_name);
    }
    return iter->second;
}

