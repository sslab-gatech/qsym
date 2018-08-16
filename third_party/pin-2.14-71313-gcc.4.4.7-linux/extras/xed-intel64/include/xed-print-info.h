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

#if !defined(_XED_PRINT_INFO_H_)
# define _XED_PRINT_INFO_H_

#include "xed-types.h"
#include "xed-decoded-inst.h"
#include "xed-disas.h" // callback function type
#include "xed-syntax-enum.h" 
#include "xed-format-options.h"

/// @ingroup PRINT
/// This contains the information used by the various disassembly printers.
/// Call xed_init_print_info to initialize the fields.  Then change the
/// required and optional fields when required.
typedef struct {

    /////////////////////////////////////////
    // REQUIRED FIELDS - users should set these
    /////////////////////////////////////////
    
    /// the decoded instruction to print
    const xed_decoded_inst_t* p;

    /// pointer to the output buffer
    char* buf;

    /// length of the output buffer. (bytes) Must be > 25 to start.
    int blen;

    /////////////////////////////////////////
    // OPTIONAL FIELDS - user can set these
    /////////////////////////////////////////
    
    /// program counter location. Must be zero if not used.  (Sometimes
    /// instructions are disassembled in a temporary buffer at a different
    /// location than where they may or will exist in memory).
    xed_uint64_t runtime_address;

    /// disassembly_callback MUST be set to zero if not used!  If zero, the
    /// default disassembly callback is used (if one has been registered).
    xed_disassembly_callback_fn_t disassembly_callback;

    /// passed to disassembly callback. Can be zero if not used.
    void* context; 

    /// default is Intel-syntax (dest on left)
    xed_syntax_enum_t syntax; 

    /// 1=indicated the format_options field is valid, 0=use default
    /// formating options from xed_format_set_options().
    int format_options_valid;  
    xed_format_options_t format_options;

    
    /////////////////////////////////////////
    // NONPUBLIC FIELDS - Users should not use these!
    /////////////////////////////////////////

    /// internal, do not use
    xed_bool_t emitted;
    
    /// internal, do not use
    unsigned int operand_indx;
    
    /// internal, do not use
    unsigned int skip_operand;

    /// internal, do not use
    xed_reg_enum_t extra_index_operand; // for MPX

} xed_print_info_t;

// This function initializes the #xed_print_info_t structure.
// You must still set the required fields of that structure.
/// @ingroup PRINT
XED_DLL_EXPORT void xed_init_print_info(xed_print_info_t* pi);

#endif
