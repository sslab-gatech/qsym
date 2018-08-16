PUBLIC IfProc1
PUBLIC IfProc2
PUBLIC IfProc3
PUBLIC IfProc4
PUBLIC IfProc5
PUBLIC IfProc6

.686
.model flat, c
.XMM

.data
extern globVal:dword

.code


IfProc1 PROC
 mov edx, 1
 xor eax, eax 
 cmp edx, ecx 
 setz al 
 ret 
IfProc1 ENDP

IfProc2 PROC
 mov edx, 1
 xor eax, eax 
 cmp edx, ecx 
 setz al 
 ret 
IfProc2 ENDP

IfProc3 PROC
 lea edx, globVal   
 mov edx, dword ptr [edx] 
 xor eax, eax 
 cmp edx, ecx 
 setz al 
 ret
IfProc3 ENDP

IfProc4 PROC
 lea eax, globVal   
 mov edx, dword ptr [eax] 
 xor eax, eax 
 cmp ecx, edx 
 setz al 
 ret
IfProc4 ENDP

IfProc5 PROC   
 mov edx, ebx
 xor eax, eax 
 cmp edx, ecx 
 setz al 
 ret 
IfProc5 ENDP

IfProc6 PROC  
 mov edx, ebx
 xor eax, eax 
 cmp dl, cl 
 setz al 
 ret 
IfProc6 ENDP

end
