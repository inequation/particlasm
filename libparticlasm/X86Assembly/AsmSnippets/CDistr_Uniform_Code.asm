	; UNIFORM COLOUR DISTRIBUTION
	movups	xmm6, [__Data.Offset{%d}]	; range[0]
	movups	xmm7, [__Data.Offset{%d} + 4 * sizeof(float)]	; range[1]
	subps	xmm7, xmm6
	; save off __ax and __dx
	push	__ax
	push	__dx
	; reserve some space on the stack - we need to lay off the original colour
	; as well as random floats
	sub		__sp, (2 * 4 * sizeof(float))
	; save off the original colour
	movups	[__sp], xmm5
	; get 4 random numbers and store them on the stack, leave one float of slack
	; (oh, the rhyme!)
	extlib	FRand, __ax
	fstp	dword [__sp + 4 * sizeof(float)]
	extlib	FRand, __ax
	fstp	dword [__sp + 5 * sizeof(float)]
	extlib	FRand, __ax
	fstp	dword [__sp + 6 * sizeof(float)]
	extlib	FRand, __ax
	fstp	dword [__sp + 7 * sizeof(float)]
	; load the random numbers into xmm
	movups	xmm5, [__sp + 4 * sizeof(float)]
	; get distribution value
	mulps	xmm5, xmm7
	addps	xmm6, xmm5
	; restore original colour in xmm5
	movups	xmm5, [__sp]
	; pop the temp variables off the stack
	add		__sp, 2 * 4 * sizeof(float)
	; restore __ax and __dx
	pop		__dx
	pop		__ax
	; mix the colours
	cdistr_mix
