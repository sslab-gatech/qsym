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



////////////////////////////////////////////////////////////////////////////

extern "C" {
#include "xed-interface.h"
#include "xed-portability.h"
#include "xed-examples-util.h"
}

#include <cstdlib>
#include <cassert>

#if defined(XED_DECODER)

////////////////////////////////////////////////////////////////////////////
static FILE*
open_file(char const* const  path, char const* const mode)
{
    FILE* f;
#if defined(XED_MSVC8_OR_LATER)
    errno_t err;
    err = fopen_s(&f,path, mode);
#else
    int err=0;
    f = fopen(path, mode);
    err = (f==0);
#endif
    if (err) {
       fprintf(stderr, "Could not open file: %s\n", path);
       exit(1);
    }
    return f;
}

static int read_byte(FILE* f, xed_uint8_t* b) {
    int  i, r;
#if defined(_WIN32)
    r = fscanf_s(f,"%2x", &i);
#else
    r = fscanf(f,"%2x", &i);
#endif
    if (b)
        *b = (xed_uint8_t)i;
    return r;
}

void
xed_disas_hex(xed_disas_info_t* fi)
{
    xed_uint8_t* region = 0;
    unsigned int len = 0;
    unsigned int i = 0;
    xed_uint8_t b = 0;
    FILE* f = 0;

    // read file once to get length
    f = open_file(fi->input_file_name, "r");
    while (read_byte(f,0) != -1)
    {
        len++;
    }
    fclose(f);

    region = (xed_uint8_t*) malloc(len);
    if (region == 0) {
        fprintf(stderr,"ERROR: Could not malloc region for hex file\n");
        exit(1);
    }

    // read file again to read the bytes
    f = open_file(fi->input_file_name, "r");
    while (read_byte(f,&b) != -1)
    {
       assert(i < len);
       region[i++] = b;
    }
    fclose(f);
    assert(i==len);

    fi->s =  (unsigned char*)region;
    fi->a = (unsigned char*)region;
    fi->q = (unsigned char*)(region) + len; // end of region
    fi->runtime_vaddr = 0;
    fi->runtime_vaddr_disas_start = 0;
    fi->runtime_vaddr_disas_end = 0;
    fi->symfn = 0;
    fi->caller_symbol_data = 0;
    fi->line_number_info_fn = 0;
    xed_disas_test(fi);
    if (fi->xml_format == 0)
        xed_print_decode_stats(fi);
}
 
////////////////////////////////////////////////////////////////////////////
#endif
