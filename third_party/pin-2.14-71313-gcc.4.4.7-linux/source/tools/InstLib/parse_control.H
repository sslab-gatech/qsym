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
#ifndef _PARSE_CONTROL_H
#define _PARSE_CONTROL_H

#include <map>
#include <iostream>
#include "control_manager.H"
#include "ialarm.H"

namespace CONTROLLER 
{
class CONTROL_CHAIN;

//this class exposes general utility functions
class PARSER
{
public:
    //convert an array of str to array of char    
    static VOID str2hex(const char* in, unsigned char* out, size_t len);
    
    //convert string to UINT32
    static UINT32 StringToUint32(const string& s);
    
    //convert string to UINT64
    static UINT64 StringToUint64(const string& s);
    
    //split string to vector, use sep as the seperator
    static unsigned int SplitArgs(const string sep, 
                                  const string& input, 
                                  vector<string>& output_array);
    
    //parse the tid token - die on fail 
    static UINT32 GetTIDToken(const string& token);
    
    //parse the tid token
    static VOID ParseTIDToken(const string& token, UINT32* tid);
    
    //parse the bcast token
    static VOID ParseBcastToken(const string& token, BOOL * bcast);
    
    //parse the count token
    static VOID ParseCountToken(const string& token, UINT32 *count);
    
    //return True whether the token is a config token(repeat, name, waitfor)
    static BOOL ConfigToken(const string& control_str);
    
    //return True if the token is "uniform"
    static BOOL UniformToken(vector<string>& tokens);
    
    //parse the config tokens and set the control chain accordingly
    static VOID ParseConfigTokens(const string& control_str, 
                                 CONTROL_CHAIN* chain);
    
    //parse the uniform tokens
    static VOID ParseUniform(const string& str, UINT32* length, UINT32* period, 
                             UINT32* repeat, UINT32* tid);

    static VOID ParseOldConfigTokens(const string& str, string& value, 
                                     string& count, string& tid, BOOL& repeat);


private:
    
    
}; //class
}; //namespace 


#endif
