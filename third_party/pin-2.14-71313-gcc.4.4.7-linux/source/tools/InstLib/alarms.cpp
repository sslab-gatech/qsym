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

#include "alarms.H"
#include "alarm_manager.H"
#include "call-stack.H"
#include <iostream>
#include <string>


using namespace std;
using namespace CONTROLLER;
using namespace CALLSTACK;

//*****************************************************************************
VOID ALARM_ICOUNT::Activate(){
    TRACE_AddInstrumentFunction(Trace, this);
}

VOID ALARM_ICOUNT::Trace(TRACE trace, VOID* v)
{
    ALARM_ICOUNT* icount_alarm = static_cast<ALARM_ICOUNT*>(v);
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        INS ins = BBL_InsHead(bbl);
        UINT32 ninst = BBL_NumIns(bbl);
        InsertIfCall_Count(icount_alarm, ins, ninst);
        InsertThenCall_Fire(icount_alarm, ins);
    }
}

//*****************************************************************************

VOID ALARM_SSC::Activate(){
    TRACE_AddInstrumentFunction(Trace, this);
}

VOID ALARM_SSC::Trace(TRACE trace, VOID* v)
{
    ALARM_SSC* ssc_alarm = static_cast<ALARM_SSC*>(v);
    UINT32 h = Uint32FromString("0x"+ssc_alarm->_ssc);
    const UINT32 pattern_len = 8;
    //the template of ssc marker
    unsigned char ssc_marker[] = { 0xbb, 0x00, 0x00, 0x00, 0x00,
                                   0x64, 0x67, 0x90};
    for(int j=0;j<4;j++){
        //fill in the ssc value
        ssc_marker[1+j]= (h>>(j*8))&0xff;
    }

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            //unsigned int instruction_size = INS_Size(ins);
            unsigned char* pc;
            pc = reinterpret_cast<unsigned char*>(INS_Address(ins));
                        
            unsigned char dst_buf[pattern_len];
            size_t copy_size = PIN_FetchCode(dst_buf, pc, pattern_len,NULL);

            if (copy_size == pattern_len &&  
              memcmp(ssc_marker,dst_buf,pattern_len) == 0){
                InsertIfCall_Count(ssc_alarm, ins, 1);
                InsertThenCall_Fire(ssc_alarm, ins);        
            }
        }
    }
}

//*****************************************************************************

VOID ALARM_ITEXT::Activate(){
    TRACE_AddInstrumentFunction(Trace, this);
}

VOID ALARM_ITEXT::Trace(TRACE trace, VOID* v)
{
    ALARM_ITEXT* itext_alarm = static_cast<ALARM_ITEXT*>(v);
    UINT32 pattern_len = itext_alarm->_itext.length();
    UINT32 pattern_bytes = pattern_len / 2; //nibbels -> bytes
    
    const size_t max_inst = 15;
    unsigned char pattern_duf[max_inst];
    PARSER::str2hex(itext_alarm->_itext.c_str(),pattern_duf,pattern_len);

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            //unsigned int instruction_size = INS_Size(ins);
            unsigned char* pc;
            pc = reinterpret_cast<unsigned char*>(INS_Address(ins));

            unsigned char dst_buf[max_inst];
            size_t copy_size = PIN_FetchCode(dst_buf, pc, pattern_bytes,NULL);

            if (copy_size == pattern_bytes &&  
                    memcmp(pattern_duf,dst_buf,pattern_bytes) == 0)
            {
                InsertIfCall_Count(itext_alarm, ins, 1);
                InsertThenCall_Fire(itext_alarm, ins);        
            }
        }
    }
}

//*****************************************************************************

VOID ALARM_INT3::Activate(){
    TRACE_AddInstrumentFunction(Trace, this);
}

VOID ALARM_INT3::Trace(TRACE trace, VOID* v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            xed_iclass_enum_t iclass;
            iclass = static_cast<xed_iclass_enum_t>(INS_Opcode(ins));
            if (iclass == XED_ICLASS_INT3){
                ALARM_INT3* int3_alarm = static_cast<ALARM_INT3*>(v);
                InsertIfCall_Count(int3_alarm, ins, 1);
                InsertThenCall_Fire(int3_alarm, ins);
                INS_Delete(ins); // so no "int3" will be actually executed
            }
        }
    }
}

//*****************************************************************************

VOID ALARM_ISA_CATEGORY::Activate(){
    TRACE_AddInstrumentFunction(Trace, this);
}

VOID ALARM_ISA_CATEGORY::Trace(TRACE trace, VOID* v)
{
    ALARM_ISA_CATEGORY* isa_ctg_alarm = static_cast<ALARM_ISA_CATEGORY*>(v);

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            xed_category_enum_t category;
            category = static_cast<xed_category_enum_t>(INS_Category(ins));
            if (category == isa_ctg_alarm->_required_ctg){
                InsertIfCall_Count(isa_ctg_alarm, ins, 1);
                InsertThenCall_Fire(isa_ctg_alarm, ins);        
            }
        }
    }
}

//*****************************************************************************

VOID ALARM_ISA_EXTENSION::Activate(){
    TRACE_AddInstrumentFunction(Trace, this);
}

VOID ALARM_ISA_EXTENSION::Trace(TRACE trace, VOID* v)
{
    ALARM_ISA_EXTENSION* isa_ext_alarm = static_cast<ALARM_ISA_EXTENSION*>(v);
    
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            xed_extension_enum_t extension;
            extension = static_cast<xed_extension_enum_t>(INS_Extension(ins));
            if (extension == isa_ext_alarm->_required_ext){
                    InsertIfCall_Count(isa_ext_alarm, ins, 1);
                    InsertThenCall_Fire(isa_ext_alarm, ins);        
            }
        }
    }
}

//*****************************************************************************

VOID ALARM_ADDRESS::Activate(){
    TRACE_AddInstrumentFunction(Trace, this);
}

VOID ALARM_ADDRESS::Trace(TRACE trace, VOID* v)
{
    ALARM_ADDRESS* address_alarm = static_cast<ALARM_ADDRESS*>(v);

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            if (INS_Address(ins) == address_alarm->_address){
                InsertIfCall_Count(address_alarm, ins, 1);
                InsertThenCall_Fire(address_alarm, ins);
            }
            
        }
    }
}

//*****************************************************************************

VOID ALARM_SYMBOL::Activate(){
    PIN_InitSymbols();
    //this is for finding the address of the required symbol
    IMG_AddInstrumentFunction(Img, this);
    TRACE_AddInstrumentFunction(Trace, this);
}

VOID ALARM_SYMBOL::Img(IMG img, VOID* v)
{
    ALARM_SYMBOL* symbol_alarm = static_cast<ALARM_SYMBOL*>(v);

    for(SYM sym = IMG_RegsymHead(img); SYM_Valid(sym); sym = SYM_Next(sym) )
    {
        string symbol = SYM_Name(sym);
        if (symbol == symbol_alarm->_symbol){
            symbol_alarm->_address = SYM_Value(sym) + IMG_LoadOffset(img); 
            return;
        }
    
    }
}
VOID ALARM_SYMBOL::Trace(TRACE trace, VOID* v)
{
    ALARM_SYMBOL* symbol_alarm = static_cast<ALARM_SYMBOL*>(v);

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            if (INS_Address(ins) == symbol_alarm->_address){
                InsertIfCall_Count(symbol_alarm, ins, 1);
                InsertThenCall_Fire(symbol_alarm, ins);
            }
        }
    }
}

//*****************************************************************************

VOID ALARM_IMAGE::Activate(){
    PIN_InitSymbols();
    //this is for finding the address of the required symbol
    IMG_AddInstrumentFunction(Img, this);
    TRACE_AddInstrumentFunction(Trace, this);
}

VOID ALARM_IMAGE::Img(IMG img, VOID* v)
{
    ALARM_IMAGE* image_alarm = static_cast<ALARM_IMAGE*>(v);
    string img_name = IMG_Name(img);
    if (img_name == image_alarm->_image){
        image_alarm->_address = IMG_LowAddress(img) + image_alarm->_offset;
        return;
    }
}

VOID ALARM_IMAGE::Trace(TRACE trace, VOID* v)
{
    ALARM_IMAGE* image_alarm = static_cast<ALARM_IMAGE*>(v);

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            if (INS_Address(ins) == image_alarm->_address){
                InsertIfCall_Count(image_alarm, ins, 1);
                InsertThenCall_Fire(image_alarm, ins);
            }
            
        }
    }
}

//*****************************************************************************

VOID ALARM_INTERACTIVE::Activate(){
#if defined(TARGET_WINDOWS)
    PIN_ERROR("interactive alarm is not supported on Windows.\n");
#endif
    _listener = _alarm_manager->GetListener();
    if (_listener == NULL){
        PIN_ERROR("interactive controller must be used "
                   "with the knob -interactive_file <file name>\n\n");
    }
    
    TRACE_AddInstrumentFunction(Trace, this);
}

VOID ALARM_INTERACTIVE::Trace(TRACE trace, VOID* v)
{
    ALARM_INTERACTIVE* alarm = static_cast<ALARM_INTERACTIVE*>(v);

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        INS ins = BBL_InsHead(bbl);
        InsertInteractiveIf(alarm, ins);
        InsertThenCall_Fire(alarm, ins);  
    }
}

VOID ALARM_INTERACTIVE::InsertInteractiveIf(ALARM_INTERACTIVE* alarm, 
                                            INS ins)
{
    INS_InsertIfCall(ins, IPOINT_BEFORE,
        AFUNPTR(InteractiveShouldFire),
        IARG_FAST_ANALYSIS_CALL,
        IARG_CALL_ORDER, alarm->GetInstrumentOrder(),
        IARG_ADDRINT, alarm,
        IARG_THREAD_ID,
        IARG_END);
}

ADDRINT PIN_FAST_ANALYSIS_CALL 
ALARM_INTERACTIVE::InteractiveShouldFire(ALARM_INTERACTIVE* alarm,
                                                     UINT32 tid)
{
    UINT32 armed = alarm->_armed[tid];
    UINT32 correct_tid = (alarm->_tid == tid) | (alarm->_tid == ALL_THREADS);

    if (armed & correct_tid){
        return alarm->_listener->CheckClearSignal();
    }
    return 0;
}

//*****************************************************************************

VOID ALARM_ENTER_FUNC::Activate(){
    CallStackManager* mngr = CallStackManager::get_instance();
    mngr->activate();
    
    //register the callback
    mngr->on_function_enter(OnFunctionStart, _func_name, this, _need_context);
}

VOID ALARM_ENTER_FUNC::OnFunctionStart(CONTEXT* ctxt, ADDRINT ip, THREADID tid, VOID *v){
    ALARM_ENTER_FUNC* alarm = static_cast<ALARM_ENTER_FUNC*>(v);
    if (alarm->_armed[tid]){
        alarm->_alarm_manager->Fire(ctxt,reinterpret_cast<VOID*>(ip),tid);
    }
    
}

//*****************************************************************************

VOID ALARM_EXIT_FUNC::Activate(){
    CallStackManager* mngr = CallStackManager::get_instance();
    mngr->activate();

    //register the callback
    mngr->on_function_exit(OnFunctionEnd, _func_name, this, _need_context);
}

VOID ALARM_EXIT_FUNC::OnFunctionEnd(CONTEXT* ctxt, ADDRINT ip, THREADID tid, VOID *v){
    ALARM_EXIT_FUNC* alarm = static_cast<ALARM_EXIT_FUNC*>(v);
    if (alarm->_armed[tid]){
        alarm->_alarm_manager->Fire(ctxt,reinterpret_cast<VOID*>(ip),tid);
    }
}

