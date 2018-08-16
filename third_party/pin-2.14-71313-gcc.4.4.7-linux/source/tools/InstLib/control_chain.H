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
#ifndef _CONTROL_CHAIN_H_
#define _CONTROL_CHAIN_H_

#include <vector>
#include <string.h>
#include "control_manager.H"


namespace CONTROLLER
{
static const UINT32 REPEAT_INDEFINITELY = 0xFFFF;

class ALARM_MANAGER; //forward deceleration

/* This class defines a single "control chain".
 * control chain is a sequence of alarms.
 * each alarm will fire the defined event and arm the next alarm 
 * in the sequence if it exists.
 * a chain can "wait" for other chains to finish by using the 
 * token waitfor:<chain_name>
 */
class CONTROL_CHAIN
{
public:
    CONTROL_CHAIN(CONTROL_MANAGER* control_mngr);
    
    //parse the chain string and create the alarm manger class and other configs
    VOID Parse(const string& chain_str);

    //set the repeat token
    VOID SetRepeat(UINT32 repeat){_repeat_token = repeat;}
    
    //set the name of the chain - will be used by the "waiting chains"
    VOID SetName(const string& name){_name = name;}
    
    //return the name of the chain
    string GetName(){return _name;}
    
    //return the Id of the chain
    UINT32 GetId(){return _id;}
    
    //set the chain Id that we are waiting for it to complete
    VOID SetWaitFor(UINT32 id);

    //set the chain name that we are waiting for it to complete
    VOID SetWaitFor(const string& chain_name);
    
    //register a waiting chine 
    VOID AddWaitingChain(CONTROL_CHAIN* chain);
    
    //return true if one of the events is a start event
    BOOL HasStartEvent();
    
    //active the first alarm, only if the chain does not waits for other chains 
    VOID Activate();
    
    //return True if we need to supply the context in the analysis functions
    BOOL NeedContext();

    //set the pointer to the uniform alarm manager in the control manager
    VOID SetUniformAlarm(ALARM_MANAGER* uniform_alarm);

    //print debug massages - only when the debug knob is used
    VOID DebugPrint();

    //call the Fire function of the control manager, which triggers all the 
    //registered control handlers
    VOID Fire(EVENT_TYPE eventID, CONTEXT* ctx, VOID * ip, 
        THREADID tid, BOOL bcast, UINT32 alarm_id);

    EVENT_TYPE EventStringToType(const string& event_name);

    UINT32 GetInsOrder(){return _control_mngr->GetInsOrder();}

    INTERACTIVE_LISTENER* GetListener(){return _control_mngr->GetListener();}
      
private:
    //calculate the next alarm in the sequence and arm it
    VOID ArmNextAlarm(UINT32 alarm_id, UINT32 tid, BOOL bcast);
    
    //when finished activate all waiting chains
    VOID ArmWaitingChains(UINT32 tid);
    
    //arm the first alarm in the chain
    VOID ArmChain(UINT32 tid);

    //arm the given alarm id
    VOID Arm(THREADID tid, BOOL bcast, UINT32 alarm_id);
    
    //return True if we need to repeat the chain for thread tid
    BOOL NeedToRepeat(UINT32 tid);
    
    //the "parent" control manager
    CONTROL_MANAGER* _control_mngr;

    //a vector of the alarms
    vector<ALARM_MANAGER*> _alarms;
    
    //count the number of repeat for each thread
    UINT32 _repeat[PIN_MAX_THREADS];
    
    //the parsed repeat token 
    UINT32 _repeat_token;
    
    //the name of the chain
    string _name;

    //the id of the chain
    UINT32 _id;

    //the id of the chain we are waiting for
    UINT32 _wait_for_id;
    
    //list of waiting chain
    list<CONTROL_CHAIN*>  _waiting_chains;

    static const UINT32 NO_WAIT = 0xFFFF;
    static UINT32 global_id;
    
}; //class 
}  //namespace
#endif
