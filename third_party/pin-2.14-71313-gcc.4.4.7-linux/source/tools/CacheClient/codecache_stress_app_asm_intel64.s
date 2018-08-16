#ifdef TARGET_MAC
#define NAME(x) _##x
#else
#define NAME(x) x
#endif

.global NAME(getStartNop), NAME(getEndNop), NAME(getBeforeFunc), NAME(getAfterFunc)
#ifndef TARGET_MAC
.type getStartNop, @function
.type getEndNop, @function
.type getBeforeFunc, @function
.type getAfterFunc, @function
#endif
NAME(getStartNop):
	lea NAME(before_nop_label)(%rip), %rax
    ret

NAME(getEndNop):
	lea NAME(after_nop_label)(%rip), %rax
	ret

NAME(getBeforeFunc):
	lea NAME(before_code_label)(%rip), %rax
	ret

NAME(getAfterFunc):
	lea NAME(after_code_label)(%rip), %rax
	ret

NAME(before_nop_label):
	nop
NAME(after_nop_label):
NAME(before_code_label):
	ret
NAME(after_code_label):
