	; UNIFORM VECTOR DISTRIBUTION
	movups	xmm6, [rel_addr(__Data.Offset{%d})]	; range[0]
	movups	xmm7, [rel_addr(__Data.Offset{%d} + 4 * sizeof(float))]	; range[1]
	subps	xmm7, xmm6
	; save off __ax and __dx
	push	__ax
	push	__dx
	; get 3 random numbers and store them on the stack, leave one float of slack
	; (oh, the rhyme!)
	sub		__sp, 4 * sizeof(float)
	extlib	FRand, __ax
	fstp	dword [__sp]
	extlib	FRand, __ax
	fstp	dword [__sp + 4]
	extlib	FRand, __ax
	fstp	dword [__sp + 8]
	; load them into xmm
	movups	xmm5, [__sp]
	; pop the temp variables off the stack
	add		__sp, 4 * sizeof(float)
	; restore __ax and __dx
	pop		__dx
	pop		__ax
	mulps	xmm5, xmm7
	addps	xmm5, xmm6
