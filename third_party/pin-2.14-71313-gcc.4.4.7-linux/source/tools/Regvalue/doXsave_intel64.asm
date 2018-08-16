PUBLIC DoXsave

extern xsaveArea:qword

.code

; void DoXsave();
; This function calls xsave and stores the FP state in the given dst area.
; The caller is expected to allocate enough space for the xsave area.
; The function expects the given dst pointer to be properly aligned for the xsave instruction.
DoXsave PROC
    ; Save the necessary GPRs
    push    rax
    push    rcx
    push    rdx

    lea     rcx, xsaveArea
    xor     rdx, rdx
    mov     rax, 7

    ; Do xsave
    xsave   [rcx]

    ; Restore the GPRs
    pop     rdx
    pop     rcx
    pop     rax
    ret
DoXsave ENDP

end
