#ifdef TARGET_MAC
#define NAME(x) _##x
#else
#define NAME(x) x
#endif

.global NAME(getStartNop), NAME(getEndNop), NAME(getBeforeFunc), NAME(getAfterFunc)
NAME(getStartNop):
	mov $NAME(before_nop_label), %eax
#ifndef TARGET_MAC
.type getStartNop, @function
.type getEndNop, @function
.type getBeforeFunc, @function
.type getAfterFunc, @function
#endif
    ret

NAME(getEndNop):
	mov $NAME(after_nop_label), %eax
	ret

NAME(getBeforeFunc):
	mov $NAME(before_code_label), %eax
	ret

NAME(getAfterFunc):
	mov $NAME(after_code_label), %eax
	ret

NAME(before_nop_label):
	nop
NAME(after_nop_label):
NAME(before_code_label):
	ret
NAME(after_code_label):
