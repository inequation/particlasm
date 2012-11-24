	; INITIAL SIZE
	; move distribution result to target register
	fldz
	fxch	st5, st0
	fstp	st0
	faddp	st5, st0
