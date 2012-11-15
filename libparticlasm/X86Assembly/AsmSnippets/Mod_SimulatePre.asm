	; SIMULATION PRE-PROCESSING
	; if the particle is not active, just skip it
	test	edx, edx
	jz		.return
	; T += t * Ts
	fld		st0
	fmul	st0, st2
	faddp	st3, st0
	; kill the particle if we're past its lifetime
	fld1
	fcomip	st3
	fwait
	; if it's still < 1, skip to the end
	jae		.end
	; decrease particle counter
	mov		__dx, [__bx + ptcEmitter.NumParticles]
	dec		__dx;
	mov		[__bx + ptcEmitter.NumParticles], __dx
	xor		__dx, __dx
.return:
	ret
.end:
