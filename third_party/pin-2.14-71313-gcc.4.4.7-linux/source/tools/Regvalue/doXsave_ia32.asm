PUBLIC DoXsave

.686
.XMM
.model flat, c
extern xsaveArea:dword

.code

; void DoXsave();
; This function calls xsave and stores the FP state in the given dst area.
; The caller is expected to allocate enough space for the xsave area.
; The function expects the given dst pointer to be properly aligned for the xsave instruction.
DoXsave PROC
    ; Save the necessary GPRs
    push    eax
    push    ecx
    push    edx

    lea     ecx, xsaveArea
    xor     edx, edx
    mov     eax, 7

    ; Do xsave
    xsave   [ecx]

    ; Restore the GPRs
    pop     edx
    pop     ecx
    pop     eax
    ret
DoXsave ENDP

end
