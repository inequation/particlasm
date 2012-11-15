	; SIZE
	; move distribution result to target register
	fldz
	fxch	st4, st0
	fstp	st0
	faddp	st4, st0
