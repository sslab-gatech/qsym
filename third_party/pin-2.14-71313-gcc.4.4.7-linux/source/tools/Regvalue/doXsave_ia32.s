
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
    push    %eax
    push    %ecx
    push    %edx

    lea     xsaveArea, %ecx
    xor     %edx, %edx
    mov     $7, %eax

    # Do xsave
    xsave   (%ecx)

    # Restore the GPRs
    pop     %edx
    pop     %ecx
    pop     %eax
    ret
