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
#ifndef _ALARM_MANAGER_H_
#define _ALARM_MANAGER_H_

#include <string>
#include <map>
#include "pin.H"
#include "ialarm.H"
#include "parse_control.H"

using namespace std;

namespace CONTROLLER
{

typedef enum { ALARM_TYPE_ICOUNT,
               ALARM_TYPE_ADDRESS,
               ALARM_TYPE_SSC,
               ALARM_TYPE_ISA_EXTENSION,
               ALARM_TYPE_ISA_CATEGORY,
               ALARM_TYPE_ITEXT,
               ALARM_TYPE_INT3,
               ALARM_TYPE_INTERACTIVE,
               ALARM_TYPE_ENTER_FUNC,
               ALARM_TYPE_EXIT_FUNC
} ALARM_TYPE;


class ALARM_MANAGER
{
public:
           
    ALARM_MANAGER(const string& alarm_str,CONTROL_CHAIN* control_chain, 
                  UINT32 id);
    
    //arm all threads in the alarm 
    VOID ArmAll(){_ialarm->Arm();}

    VOID Activate();

    //arm the alarm for thread id 
    VOID ArmTID(THREADID tid){_ialarm->Arm(tid);}
    
    //return True if the alarm raises start event
    BOOL HasStartEvent();
    
    //rolls the event to the control chain
    VOID Fire(CONTEXT* ctx, VOID * ip, THREADID tid);
    
    BOOL ArmNext(){return _arm_next;}

    THREADID GetTid(){return _tid;}

    BOOL IsUniformDone();

    //print the alarm - for debug
    VOID Print();

    UINT32 GetInsOrder(){return _control_chain->GetInsOrder();}

    INTERACTIVE_LISTENER* GetListener(){return _control_chain->GetListener();}
    
private:  
    //extract the event id
    VOID ParseEventId(vector<string>& control_tokens);
    
    //extract the alarm name and value
    VOID ParseAlarm(vector<string>& control_tokens);
    
    //parse common alarm configs(tid,bcast,count)
    VOID ParseCommon(vector<string>& control_tokens);

    //parse the "uniform" token
    VOID ParseUniform(vector<string>& control_tokens);
    VOID SetNextUniformEvent(THREADID tid);
    VOID Disarm(THREADID tid);
    VOID Disarm();
    
    //generate address type alarm(address, symbol, image)
    IALARM* GenAddress();

    IALARM* GenUniform();
    
    //generate an alarm
    IALARM* GenerateAlarm();

    //define the mapping of alarm name to ALARM_TYPE
    map<string, ALARM_TYPE> InitAlarms();
    
    //return the type of the alarm
    ALARM_TYPE GetAlarmType(const string& alarm_name);

    //full string of the alarm before parsing
    string _raw_alarm;

    //the name of the event (start,stop, ...) 
    string _event_name;
    
    //the type of the event
    EVENT_TYPE _event_type;
    
    //the name of the alarm(icount,ssc-mark,...)
    string _alarm_name;

    ALARM_TYPE _alarm_type;
    
    //the value of the alarm
    string _alarm_value;
    
    //indicates whether we need to broadcast the event to all the threads
    BOOL _bcast;

    //the thread id of the alarm
    UINT32 _tid;
    
    //fire only after #count
    UINT32 _count;

    UINT32 _id;
    
    BOOL _uniform_type;
    UINT64 _uniform_length;
    UINT64 _uniform_period;
    UINT64 _uniform_count;
    BOOL _arm_next;

       
    
    //the generic alarm object - all the concrete alarm inherits from this class
    IALARM* _ialarm;
    CONTROL_CHAIN* _control_chain;
    
    map<string, ALARM_TYPE> _alarm_map;

};//class
}//namespace
#endif
