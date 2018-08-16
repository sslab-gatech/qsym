.global numthreadsStarted
.type AtomicIncrement, @function
.global AtomicIncrement
AtomicIncrement:
    lea     numthreadsStarted(%rip), %rcx
    incl     (%rcx)
    ret



