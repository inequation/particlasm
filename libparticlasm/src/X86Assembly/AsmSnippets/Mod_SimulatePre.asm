	; SIMULATION PRE-PROCESSING
	; T += t * Ts
	fld		st0			; duplicate the time step
	fmul	st0, st3	; multiply by time step
	faddp	st4, st0	; add scaled time step to lifetime
	; kill the particle if we're past its lifetime
	fld1
	fcomip	st3
	fwait
	; if it's still < 1, skip to the end
	jae		.end
	; mark as inactive
	xor		edx, edx
	return
.end:
