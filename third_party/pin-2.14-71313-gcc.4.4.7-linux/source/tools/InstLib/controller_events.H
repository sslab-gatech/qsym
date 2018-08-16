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
#ifndef _CONTROLLER_EVENTS_H_
#define _CONTROLLER_EVENTS_H_

#include <string>
#include <map>
#include "pin.H"

using namespace std;


namespace CONTROLLER
{

typedef enum{
        EVENT_INVALID = 0,
        EVENT_PRECOND,
        EVENT_START,
        EVENT_STOP,
        EVENT_THREADID,
        EVENT_WARMUP_START,
        EVENT_WARMUP_STOP,
        EVENT_PROLOG_START,
        EVENT_PROLOG_STOP,
        EVENT_EPILOG_START,
        EVENT_EPILOG_STOP,
        EVENT_STATS_RESET,
        EVENT_STATS_EMIT,
        EVENT_USER_0,
        EVENT_USER_1,
        EVENT_USER_2,
        EVENT_USER_3,
        EVENT_USER_4,
        EVENT_USER_5,
        EVENT_USER_6,
        EVENT_USER_7,
        EVENT_USER_8,
        EVENT_USER_9
    }EVENT_TYPE;

//the events manager can be created only by the control object
class CONTROLLER_EVENTS
{
public:
    EVENT_TYPE AddEvent(const string& event_name);
    string IDToString(EVENT_TYPE ev);
    EVENT_TYPE EventStringToType(const string& event_name);
    
    
private:
    friend class CONTROL_MANAGER;
    CONTROLLER_EVENTS();     
    map<string,EVENT_TYPE> _events;
    static const EVENT_TYPE _max_user_ev = EVENT_USER_9;
    
    
    
};//class
}//namespace
#endif
