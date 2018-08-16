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

#if !defined(_EMX_CALL_STACK_H_)
#define _EMX_CALL_STACK_H_

#include <string>
#include <vector>
#include <map>
#include <set>
#include "pin.H"
extern "C"{
#include "xed-interface.h"
}

namespace CALLSTACK{
class CallEntry {
private:
    ADDRINT _current_sp;
    ADDRINT _target;

public:
    CallEntry(): _current_sp(0),_target(0) { }
    CallEntry(ADDRINT current_sp, ADDRINT target):
        _current_sp(current_sp),
        _target(target)
    { }

    bool operator==( const CallEntry& a) const {
        return(_current_sp == a._current_sp);
    }
    ADDRINT sp() const {return _current_sp;}
    ADDRINT target() const {return _target;}
};


class CallStack {
public:
    // print the call stack, emit only 'depth' entries
    void emit_stack(UINT32 depth, vector<string>& out);
    
    // return the depth of the call stack
    UINT32 depth();
    
    // add the current_sp and the target to the top of the call stack
    void push_head(ADDRINT current_sp, ADDRINT target);
    
    // return the ip target of the latest call
    ADDRINT top_target();
    
    // capture the info for each ip in the call stack
    // see CallStackInfo 
    void save_all_ips_info();

    void process_call(ADDRINT current_sp, ADDRINT target);
    void process_return(ADDRINT current_sp, ADDRINT ip);
private:
    typedef std::vector<CallEntry> CallVec;
    CallVec _call_vec;

    void create_entry(ADDRINT current_sp, ADDRINT target);
    void adjust_stack( ADDRINT current_sp);
};

typedef void (*CALL_STACK_HANDLER)(CONTEXT* ctxt, ADDRINT ip, THREADID tid, VOID *v);
class CallStackHandlerParams{
public:
    CallStackHandlerParams(CALL_STACK_HANDLER h, const string& func_name, void* v){
        _handler = h;
        _function_name = func_name;
        _args = v;
    }
    
    CALL_STACK_HANDLER _handler;
    string _function_name;
    void* _args;
};


// this struct holds the informations need for emitting the call stack
// we hold a map of ip->CallStackInfo so we will no 
// generate info for the same ip more than once
typedef struct {
    string func_name;
    string image_name;
    string file_name;
    INT32 line;
    INT32 column;
} CallStackInfo;


// a singleton class 
class CallStackManager{
public:
    // return a pointer to an instance of the class
    static CallStackManager* get_instance();
    
    // return a copied CallStack of thread tid
    CallStack get_stack(THREADID tid);
    
    // activate the CallStackManager
    void activate();
    
    // fill in info with the information about the ip, see CallStackInfo
    // if the the info does not exists we generated it first
    void get_ip_info(ADDRINT ip, CallStackInfo& info);

    //register a callback the will be called when entering to function: func_name
    void on_function_enter(CALL_STACK_HANDLER handler, const string& func_name, void* v, BOOL use_ctxt);
    
    //register a callback the will be called when returning from function: func_name
    void on_function_exit(CALL_STACK_HANDLER handler, const string& func_name, void* v, BOOL use_ctxt);

    //// internal use ////
    void on_call(THREADID tid, CONTEXT* ctxt, ADDRINT ip);
    void on_ret_fire(THREADID tid, CONTEXT* ctxt, ADDRINT ip);
    BOOL on_ret_should_fire(THREADID tid);
    BOOL NeedContext();
    BOOL TargetInteresting(ADDRINT ip);

private:
    CallStackManager(): _activated(false),_use_ctxt(false),
        _depth_func_handlers_tid_vec(PIN_MAX_THREADS){
        PIN_InitLock(&_lock);
    }
    static void thread_begin(THREADID tid, CONTEXT* ctxt,
                             INT32 flags, void* v);
    void add_stack(THREADID tid, CallStack* call_stack);
    static void Img(IMG img, void* v);

    static CallStackManager* _instance;
    bool _activated;
    //a map to threadid -> CallStack*
    typedef std::map<THREADID, CallStack*> CallStackMap;
    CallStackMap _call_stack_map;

    //map of ip to its info(file, func, line, ...)
    //used to prevent collecting info about the same ip multiple times 
    typedef std::map<ADDRINT, CallStackInfo> CallStackInfoMap;
    CallStackInfoMap _call_stack_info;
    PIN_LOCK _lock;
    BOOL _use_ctxt;
    
    vector<CallStackHandlerParams> _enter_func_handlers;
    vector<CallStackHandlerParams> _exit_func_handlers;

    //map of ip to a vector of handlers
    typedef vector<CallStackHandlerParams*> CallStackHandlerVec;
    typedef map<ADDRINT, CallStackHandlerVec> IpFuncHnadlersMap;
    IpFuncHnadlersMap _enter_func_handlers_map;
    
    //map of ip to a vector of handlers
    IpFuncHnadlersMap _exit_func_handlers_map;

    //map of stack depth to a vector of handlers
    typedef std::map<UINT32, CallStackHandlerVec> DepthFuncHandlersMap;
    typedef std::vector<DepthFuncHandlersMap> DepthFuncHandlersTidVec;
    //a vector with entry per thread 
    DepthFuncHandlersTidVec _depth_func_handlers_tid_vec;

    //holds the ips that we have marked for exit, needed for recursive calls
    set<ADDRINT> _marked_ip_for_exit;
    
    
};
}  //namespace
#endif
