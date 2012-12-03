	; COLOUR
	; load colour flags
	mov		eax, [rel_addr(__Data.Offset{%d})]
	; load previous colour (colour distributions require this)
	movaps	xmm5, xmm1
