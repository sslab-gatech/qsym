PUBLIC getStartNop
PUBLIC getEndNop
PUBLIC getBeforeFunc
PUBLIC getAfterFunc

.686
.model flat, c
.XMM

.code


getStartNop PROC
    mov eax, offset before_nop_label
	ret
getStartNop ENDP

getEndNop PROC
    mov eax, offset after_nop_label
	ret
getEndNop ENDP

getBeforeFunc PROC
    mov eax, offset before_code_label
	ret
getBeforeFunc ENDP

getAfterFunc PROC
    mov eax, offset after_code_label
	ret
getAfterFunc ENDP

before_nop_label:
	nop
after_nop_label:
before_code_label:
	ret
after_code_label:


end
