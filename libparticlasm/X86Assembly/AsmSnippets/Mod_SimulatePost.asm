	; v += a * t
	movaps	xmm7, xmm4
	mulps	xmm7, xmm0
	addps	xmm3, xmm7
	; x += v * t
	movaps	xmm7, xmm3
	mulps	xmm7, xmm0
	addps	xmm2, xmm7
	ret
