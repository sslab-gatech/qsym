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
/*
 * This header file consolidate definitions for OS X* and Linux
 * running on both ia32 and intel64 architecture.
 */

/*-------------------------------------------------------------------
 * OS specific macros:
 *-------------------------------------------------------------------
 * NAME(symbol)              - Decorate 'symbol' as a global symbol
 * DECLARE_FUNCTION(symbol)  - Declare 'symbol' as function type
 *-------------------------------------------------------------------
 */
#ifdef TARGET_MAC
# define NAME(x) _##x
# define DECLARE_FUNCTION(fun)
#else
# define NAME(x) x
# define DECLARE_FUNCTION(fun) .type fun,  @function
#endif

/*-------------------------------------------------------------------
 * Architecture specific macros:
 *-------------------------------------------------------------------
 * BEGIN_STACK_FRAME         - Expands to the instructions that build a new stack
 *                             frame for a function
 * END_STACK_FRAME           - Expands to the instructions that destroy a stack
 *                             frame just before calling "ret"
 * PARAM1                    - The first argument to a function.
 *                             Note that this macro is valid only after building a
 *                             stack frame
 * RETURN_REG                - The register that holds the return value
 * STACK_PTR                 - The stack pointer register
 * PIC_VAR(v)                - Reference memory at 'v' in PIC notation (not supported in 32 bit mode)
 *-------------------------------------------------------------------
 */
#if defined(TARGET_IA32)
# define BEGIN_STACK_FRAME \
             push %ebp; \
             mov %esp, %ebp
# define END_STACK_FRAME \
             mov %ebp, %esp; \
             pop %ebp
# define PARAM1 8(%ebp)
# define RETURN_REG %eax
# define STACK_PTR %esp
# define PIC_VAR(a) a
#elif defined(TARGET_IA32E) || defined(TARGET_MIC)
# define BEGIN_STACK_FRAME \
             push %rbp; \
             mov %rsp, %rbp
# define END_STACK_FRAME \
             mov %rbp, %rsp; \
             pop %rbp
# define PARAM1 %rdi
# define RETURN_REG %rax
# define STACK_PTR %rsp
# define PIC_VAR(a) a(%rip)
#endif
