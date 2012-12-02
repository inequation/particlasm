	; UNIFORM SCALAR DISTRIBUTION
	fld		float_s [rel_addr(__Data.Offset{%d})]	; range[0]
	fld		float_s [rel_addr(__Data.Offset{%d} + sizeof(float))]	; range[1]
	; exchange the registers - FPU is stupid...
	fxch	st1
	; save off __ax and __dx
	push	__ax
	push	__dx
	; temp variable
	sub		__sp, sizeof(float)
	; st1=range[1], st0=range[0]
	; range[1] - range[0]
	fsub	st1, st0
	; save range[0] to temp var
	fstp	float_s [__sp]
	; st0=range[1] - range[0]
	extlib	FRand, __ax
	; multiply the range span by the random number
	; st0=frand(), st1=range[1] - range[0]
	fmulp	st1, st0
	; add the lower bound
	; st0=frand() * (range[1] - range[0])
	fld		float_s [__sp]
	faddp	st1, st0
	add		__sp, sizeof(float)
	; restore __ax and __dx
	pop		__dx
	pop		__ax
	; st0=distribution result
