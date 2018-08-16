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


#include "xed-interface.h"
#include <stdio.h>

int main(int argc, char** argv);

int main(int argc, char** argv)
{
    xed_bool_t long_mode = 1;
    xed_decoded_inst_t xedd;
    xed_state_t dstate;
    unsigned char itext[15] = { 0xf2, 0x2e, 0x4f, 0x0F, 0x85, 0x99,
                                0x00, 0x00, 0x00 };

    xed_tables_init(); // one time per process

    if (long_mode) 
        dstate.mmode=XED_MACHINE_MODE_LONG_64;
    else 
        dstate.mmode=XED_MACHINE_MODE_LEGACY_32;

    xed_decoded_inst_zero_set_mode(&xedd, &dstate);
    xed_ild_decode(&xedd, itext, XED_MAX_INSTRUCTION_BYTES);
    printf("length = %d\n",xed_decoded_inst_get_length(&xedd));
    
    return 0;
    (void) argc; (void) argv; //pacify compiler

}
