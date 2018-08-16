; This header file consolidate definitions for Windows
; running on both ia32 and intel64 architecture.
; Note that all definitions here are according to the 'cdecl'
; calling convention (the default in Windows)

;-------------------------------------------------------------------
; Architecture specific macros:
;-------------------------------------------------------------------
; PROLOGUE                  - The prologue of the program - must be placed before
;                             any other declaration
; BEGIN_STACK_FRAME         - Expands to the instructions that build a new stack
;                             frame for a function
; END_STACK_FRAME           - Expands to the instructions that destroy a stack
;                             frame just before calling "ret"
; PARAM1                    - The first argument to a function.
;                             Note that this macro is valid only after building a
;                             stack frame
; RETURN_REG                - The register that holds the return value
; NATIVE_SIZE_SUFFIX <inst> - Add the suffix representing the word size native to
;                             the current target to instruction <inst>.
;                             For example "NATIVE_SIZE_SUFFIX popf" will be expanded
;                             to popfd on ia32, and popfq on intel64.
; STACK_PTR                 - The stack pointer register
; ADDRINT_PTR               - Prefix for memory operands to refer to sizeof(ADDRINT)
;-------------------------------------------------------------------

IFDEF TARGET_IA32
    PROLOGUE MACRO
        .686
        .model flat, c
    ENDM
    BEGIN_STACK_FRAME macro
        push ebp
        mov ebp,esp
    ENDM
    END_STACK_FRAME macro
        mov esp,ebp
        pop ebp
    ENDM
    PARAM1 textequ <[EBP+8]>
    RETURN_REG textequ <EAX>
    STACK_PTR textequ <ESP>
    ADDRINT_PTR textequ <DWORD PTR>
	NATIVE_SIZE_SUFFIX macro inst
	    &inst&d
	ENDM
ELSE
    PROLOGUE MACRO
    ENDM
    BEGIN_STACK_FRAME macro
        push rbp
        mov rbp,rsp
    ENDM
    END_STACK_FRAME macro
        mov rsp,rbp
        pop rbp
    ENDM
    PARAM1 textequ <RCX>
    RETURN_REG textequ <RAX>
    STACK_PTR textequ <RSP>
    ADDRINT_PTR textequ <QWORD PTR>
	NATIVE_SIZE_SUFFIX macro inst
	    &inst&q
	ENDM
ENDIF
