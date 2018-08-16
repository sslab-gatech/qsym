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

#include "parse_control.H"
#include "control_chain.H"
#include "util/strings.hpp"

using namespace std;

namespace CONTROLLER
{
unsigned int PARSER::SplitArgs(const string sep, 
                                const string& input, 
                                vector<string>& output_array)
 {
    // returns the number of args
    // rip off the separator characters and split the src 
    // string based on separators.

    // find the string between last_pos and pos. pos is after last_pos
    size_t last_pos = input.find_first_not_of(sep, 0);
    size_t pos = input.find_first_of(sep, last_pos);  
    
    int i=0;
    while( pos != string::npos && last_pos != string::npos ) 
    {
        string a = input.substr(last_pos, pos-last_pos);
        output_array.push_back(a); 
        
        last_pos = input.find_first_not_of(sep, pos);
        pos = input.find_first_of(sep, last_pos);  
        i++;
    }
    if (last_pos != string::npos && pos == string::npos)
    {
        
        string a = input.substr(last_pos); // get the rest of the string
        output_array.push_back(a);
        i++;
    }
    return i;
}

BOOL PARSER::ConfigToken(const string& control_str){
    vector<string> tokens;
    SplitArgs(":",control_str,tokens);
    if (tokens[0] == "repeat" ||
        tokens[0] == "name" ||
        tokens[0] == "waitfor"){
            return TRUE;
    }
    return FALSE;   
}

BOOL PARSER::UniformToken(vector<string>& tokens){
    if (tokens[0] == "uniform"){
            return TRUE;
    }
    return FALSE; 
}
VOID PARSER::ParseConfigTokens(const string& control_str, CONTROL_CHAIN* chain){
    vector<string> tokens;
    SplitArgs(":",control_str,tokens);
    if (tokens[0] == "repeat"){
        if (tokens.size() == 1){
            chain->SetRepeat(REPEAT_INDEFINITELY);
        }
        else{
            UINT32 repeat = StringToUint32(tokens[1]);
            chain->SetRepeat(repeat);
        }
    }
    else if (tokens[0] == "name"){
        chain->SetName(tokens[1]);
    }
    else if (tokens[0] == "waitfor"){
        chain->SetWaitFor(tokens[1]);
    }
    else {
        ASSERT(FALSE,"Unexpected config token");
    }
}

VOID PARSER::ParseBcastToken(const string& str, BOOL* bcast){
    string bcast_str = "bcast";

    if (str.compare(0,bcast_str.length(),bcast_str) == 0){
        *bcast = TRUE;
    }
}

UINT32 PARSER::GetTIDToken(const string& str){
    string tid_str = "tid";

    if (str.compare(0,tid_str.length(),tid_str) == 0){
        string s = str.substr(tid_str.length());
        return StringToUint32(s);
    }
    ASSERT(FALSE,"failed to parse tid token");
    return 0;
}


VOID PARSER::ParseTIDToken(const string& str, UINT32* tid){
    string tid_str = "tid";

    if (str.compare(0,tid_str.length(),tid_str) == 0){
        string s = str.substr(tid_str.length());
        *tid = StringToUint32(s);
    }
}

VOID PARSER::ParseCountToken(const string& str, UINT32* count){
    string count_str = "count";
    
    if (str.compare(0,count_str.length(),count_str) == 0){
        string c = str.substr(count_str.length());
        ASSERT(!c.empty(),"count must have a numeric value");
        *count = StringToUint32(c);
    }
}

VOID PARSER::ParseUniform(const string& str, UINT32* length, UINT32* period, 
                          UINT32* repeat, UINT32* tid)
{
    vector<string> tokens;
    SplitArgs(":",str,tokens);
    ASSERT(tokens[0] == "uniform","");
    string error = "uniform usage error: "
        "uniform:<length>:<period>:<repeat>[:tid<id>]";
    if(tokens.size() != 4 && tokens.size() != 5){
        ASSERT(FALSE,error);
    }

    *length = StringToUint32(tokens[1]);
    *period = StringToUint32(tokens[2]);
    *repeat = StringToUint32(tokens[3]);

    if(tokens.size() == 5){
        *tid = GetTIDToken(tokens[4]);
    }
    else{
       *tid = ALL_THREADS;
    }
}

UINT32 PARSER::StringToUint32(const string& s){
    UINT32 val = 0;
    string::const_iterator it;
    it = UTIL::ParseUnsigned(s.begin(), s.end(), 0, &val);
    
    if (it != s.end())
        ASSERT(FALSE,"failed converting string to int:" + s);

    return val;
}

UINT64 PARSER::StringToUint64(const string& s){
    UINT64 val = 0;
    string::const_iterator it;
    it = UTIL::ParseUnsigned(s.begin(), s.end(), 0, &val);

    if (it != s.end())
        ASSERT(FALSE,"failed converting string to int:" + s);

    return val;
}

static UINT8 convert_nibble(UINT8 n) {
    if (n >= '0' && n <= '9') return n -'0';
    if (n >= 'a' && n <= 'f') return n -'a'+10;
    if (n >= 'A' && n <= 'F') return n -'A'+10;
    cerr << "Bad nibble in hex string: " << (char)n << endl;
    ASSERTX(0);
    return 0;
}

VOID PARSER::str2hex(const char* in, unsigned char* out, size_t len) {
    size_t i=0, j=0;
    for(i=0;i<len;i+=2) 
        out[j++] = convert_nibble(in[i])*16+ convert_nibble(in[i+1]);
}

VOID PARSER::ParseOldConfigTokens(const string& str, string& value, 
                                  string& count, string& tid, BOOL& repeat)
{
    //parsing old controller tokens: [:<int>][:tid<tid>][:repeat]
    vector<string> tokens;
    SplitArgs(":",str,tokens);
    value = tokens[0];
    if (tokens.size() > 1){
        for (UINT32 i = 1; i < tokens.size(); i++){
            if (tokens[i] == "repeat"){
                    repeat = TRUE;
            }
            else if (tokens[i].substr(0,3) == "tid"){
                tid = tokens[i];
            }
            else{
                //this must be a count token -> simple int 
                count = tokens[i];
            }
        }
    }
}


}; //namespace 

