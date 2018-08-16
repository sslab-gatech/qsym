
.data
.extern xsaveArea

.text

# void DoXsave();
# This function calls xsave and stores the FP state in the given dst area.
# The caller is expected to allocate enough space for the xsave area.
# The function expects the given dst pointer to be properly aligned for the xsave instruction.
#ifndef TARGET_MAC
.type DoXsave,  @function
#endif
.global DoXsave
DoXsave:
    # Save the necessary GPRs
    push    %rax
    push    %rcx
    push    %rdx

    lea     xsaveArea(%rip), %rcx
    xor     %rdx, %rdx
    mov     $7, %rax

    # Do xsave
    xsave   (%rcx)

    # Restore the GPRs
    pop     %rdx
    pop     %rcx
    pop     %rax
    ret
