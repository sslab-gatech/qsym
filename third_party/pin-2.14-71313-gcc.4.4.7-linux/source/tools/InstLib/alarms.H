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
#ifndef _ALARMS_H_
#define _ALARMS_H_

#include "ialarm.H"
#include "parse_control.H"
extern "C" {
#include "xed-interface.h"
}

//in this file we define all standard alarms
//each alarm inherits form IALRM
namespace CONTROLLER 
{
//*****************************************************************************
class ALARM_ICOUNT : public IALARM
{
public:
    ALARM_ICOUNT(const string& icount_str, UINT32 tid, UINT32 count, 
                 BOOL need_ctxt, ALARM_MANAGER* manager):
      IALARM(tid, count, need_ctxt, manager){
          UINT64 icount = PARSER::StringToUint64(icount_str);
          //for icount alarm the affective count is the a combination
          //of the icount value and the count value
          SetCount(icount*count);
          Activate();
      }


private:
    VOID Activate();
    static VOID Trace(TRACE trace, VOID* v);
};

//*****************************************************************************

class ALARM_ADDRESS : public IALARM
{
public:
    ALARM_ADDRESS(const string& address, UINT32 tid, UINT32 count, 
                 BOOL need_ctxt, ALARM_MANAGER* manager):      
      IALARM(tid, count, need_ctxt, manager){
          _address = PARSER::StringToUint64(address);
          Activate();
      }
private:
    VOID Activate();
    static VOID Trace(TRACE trace, VOID* v);

    ADDRINT _address;
};
//*****************************************************************************
class ALARM_SYMBOL : public IALARM
{
public:
    ALARM_SYMBOL(const string& symbol, UINT32 tid, UINT32 count, 
                 BOOL need_ctxt, ALARM_MANAGER* manager):
      IALARM(tid, count, need_ctxt, manager), _symbol(symbol){
        Activate();
      }

private:
    VOID Activate();
    static VOID Img(IMG img, VOID* v);
    static VOID Trace(TRACE trace, VOID* v);

    string _symbol;
    ADDRINT _address;
};

//*****************************************************************************

class ALARM_IMAGE : public IALARM
{
public:
    ALARM_IMAGE(const string& image, const string& offset , UINT32 tid, 
                UINT32 count, BOOL need_ctxt, ALARM_MANAGER* manager): 
    IALARM(tid, count, need_ctxt, manager),
    _image(image){
        _offset = PARSER::StringToUint64(offset);    
        Activate();
    }

private:
    VOID Activate();
    static VOID Img(IMG img, VOID* v);
    static VOID Trace(TRACE trace, VOID* v);

    string _image;
    ADDRINT _address;
    UINT64 _offset;
};


//*****************************************************************************
class ALARM_SSC : public IALARM
{
public:
    ALARM_SSC(const string& ssc, UINT32 tid, UINT32 count, 
                 BOOL need_ctxt, ALARM_MANAGER* manager):
      IALARM(tid, count, need_ctxt, manager),_ssc(ssc){
          Activate();
      }

private:
    string _ssc;    
    static const UINT32 _pattern_len = 8;
    
    VOID Activate();
    static VOID Trace(TRACE trace, VOID* v);
};

//*****************************************************************************
class ALARM_ITEXT : public IALARM
{
public:
    ALARM_ITEXT(const string& itext, UINT32 tid, UINT32 count, 
        BOOL need_ctxt, ALARM_MANAGER* manager):
    IALARM(tid, count, need_ctxt, manager),_itext(itext){
        Activate();
    }


private:
    string _itext;
    VOID Activate();
    static VOID Trace(TRACE trace, VOID* v);
};

//*****************************************************************************
class ALARM_INT3 : public IALARM
{
public:
    ALARM_INT3(const string& icount_str,UINT32 tid, UINT32 count, 
        BOOL need_ctxt, ALARM_MANAGER* manager): 
    IALARM(tid, count, need_ctxt, manager){
            UINT64 icount = PARSER::StringToUint64(icount_str);
            //the count is the a combination 
            //of the icount value and the count value
            SetCount(icount*count);
            Activate();
    }


private:
    VOID Activate();
    static VOID Trace(TRACE trace, VOID* v);
};

//*****************************************************************************

class ALARM_ISA_CATEGORY : public IALARM
{
public:
    ALARM_ISA_CATEGORY(const string& isa_ctg, UINT32 tid, UINT32 count, 
        BOOL need_ctxt, ALARM_MANAGER* manager):
    IALARM(tid, count, need_ctxt, manager){
        _isa_ctg = isa_ctg;
        _required_ctg = str2xed_category_enum_t(_isa_ctg.c_str());
        Activate();
    }

private:
    string _isa_ctg;
    xed_category_enum_t _required_ctg;
    VOID Activate();
    static VOID Trace(TRACE trace, VOID* v);
};

//*****************************************************************************
class ALARM_ISA_EXTENSION : public IALARM
{
public:
    ALARM_ISA_EXTENSION(const string& isa_ext, UINT32 tid, UINT32 count, 
                        BOOL need_ctxt, ALARM_MANAGER* manager):
    IALARM(tid, count, need_ctxt, manager){
          _isa_ext = isa_ext;
          _required_ext = str2xed_extension_enum_t(_isa_ext.c_str());
          Activate();
      }


private:
    string _isa_ext;
    xed_extension_enum_t _required_ext;
    VOID Activate();
    static VOID Trace(TRACE trace, VOID* v);
};

//*****************************************************************************
class ALARM_INTERACTIVE : public IALARM
{
public:
    ALARM_INTERACTIVE(UINT32 tid, BOOL need_ctxt, 
                      ALARM_MANAGER* manager):
    IALARM(tid, 1, need_ctxt, manager){
        Activate();
      }

private:
    INTERACTIVE_LISTENER* _listener;

    VOID Activate();
    static VOID Trace(TRACE trace, VOID* v);
        
    //add if analysis function
    static VOID InsertInteractiveIf(ALARM_INTERACTIVE* alarm, INS ins);

    static ADDRINT PIN_FAST_ANALYSIS_CALL 
        InteractiveShouldFire(ALARM_INTERACTIVE* alarm, UINT32 tid);
};

//*****************************************************************************
class ALARM_ENTER_FUNC : public IALARM
{
public:
    ALARM_ENTER_FUNC(const string& func, UINT32 tid, UINT32 count, 
                     BOOL need_ctxt, ALARM_MANAGER* manager):
    IALARM(tid, count, need_ctxt, manager){
        _func_name = func;
        Activate();
    }


private:
    string _func_name;

    VOID Activate();
    static VOID OnFunctionStart(CONTEXT* ctxt, ADDRINT ip, THREADID tid, VOID *v);
};

//*****************************************************************************

class ALARM_EXIT_FUNC : public IALARM
{
public:
    ALARM_EXIT_FUNC(const string& func, UINT32 tid, UINT32 count, 
                    BOOL need_ctxt, ALARM_MANAGER* manager):
    IALARM(tid, count, need_ctxt, manager){
        _func_name = func;
        Activate();
    }

private:
    string _func_name;
    
    VOID Activate();
    static VOID OnFunctionEnd(CONTEXT* ctxt, ADDRINT ip, THREADID tid, VOID *v);
};



} //namespace
#endif
