PUBLIC IfProc1
PUBLIC IfProc2
PUBLIC IfProc3
PUBLIC IfProc4
PUBLIC IfProc5
PUBLIC IfProc6
extern globVal:dword

.code


IfProc1 PROC
 mov r8, 1
 xor eax, eax 
 cmp r8, rcx 
 setz al 
 ret 
IfProc1 ENDP

IfProc2 PROC
 mov r8, 1
 xor eax, eax 
 cmp r8d, ecx 
 setz al 
 ret 
IfProc2 ENDP

IfProc3 PROC
 lea    rdx, globVal   
 mov r8d, dword ptr [rdx] 
 xor eax, eax 
 cmp r8d, ecx 
 setz al 
 ret
IfProc3 ENDP

IfProc4 PROC
 lea    rax, globVal   
 mov r8d, dword ptr [rax] 
 xor eax, eax 
 cmp ecx, r8d 
 setz al 
 ret
IfProc4 ENDP

IfProc5 PROC   
 mov r8, r14
 xor eax, eax 
 cmp r8d, ecx 
 setz al 
 ret 
IfProc5 ENDP

IfProc6 PROC  
 mov rdx, rbx
 xor eax, eax 
 cmp dl, cl 
 setz al 
 ret 
IfProc6 ENDP

end
