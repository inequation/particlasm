; particlasm value distributions code
; Copyright (C) 2011-2012, Leszek Godlewski <lg@inequation.org>

%ifndef PTC_DISTRIBUTIONS
%define PTC_DISTRIBUTIONS

; scalar distributions put their result into st0
; vector and colour - into xmm5
; NOTE: colour distributions expect input colour in xmm5 and flags in __ax up
; front!

; Compilation calling convention:
; __cx - pointer to working code buffer
; __di - pointer to working data buffer

; Other than that, same calling conventions apply for spawning and processing as
; for modules.

; ========================================
; Scalar Constant
; ========================================

sdistr_Constant:
	fld		float_s [dummy_addr]	; val; this address is filled upon compilation
.end:
sdistr_Constant_size		equ	(sdistr_Constant.end - sdistr_Constant)
sdistr_ConstantData_size	equ	4	; one 32-bit float

sdistr_ConstantCompile:
	; first, copy data to data buffer
	mov		edx, [__si + ptcSDistr_Constant.Val]
	mov		[__di], edx

	; save off __cx
	push	__cx

	; now, memcpy the code
	push	size_t sdistr_Constant_size
%ifidn __BITS__,64
	lea		__ax, [rel sdistr_Constant]
	push	__ax
%else
	push	ptr_t sdistr_Constant
%endif
	push	__cx
	call	memcpy wrt ..plt
	add		__sp, sizeof(size_t) + 2 * sizeof(ptr_t)

	; restore __cx
	pop		__cx

	; fix the fld instruction operand
	mov		[__cx + sdistr_Constant.end - sdistr_Constant - sizeof(ptr_t)], __di

	; advance the pointers
	add		__cx, sdistr_Constant_size
	add		__di, sdistr_ConstantData_size
	ret

; ========================================
; Scalar Uniform
; ========================================

sdistr_Uniform:
	fld		float_s [dummy_addr]	; range[0]; this address is filled upon compilation
.postrange0:
	fld		float_s [dummy_addr]	; range[1]; this address is filled upon compilation
.postrange1:
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
	fwait
	frand	2 * sizeof(ptr_t) + sizeof(float)
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
.end:
sdistr_Uniform_size		equ	(sdistr_Uniform.end - sdistr_Uniform)
sdistr_UniformData_size	equ	8

sdistr_UniformCompile:
	; first, copy data to data buffer
	mov		edx, [__si + ptcSDistr_Uniform.Range]
	mov		[__di], edx
	mov		edx, [__si + ptcSDistr_Uniform.Range + sizeof(float)]
	mov		[__di + sizeof(float)], edx

	; save off __cx
	push	__cx

	; now, memcpy the code
	push	size_t sdistr_Uniform_size
%ifidn __BITS__,64
	lea		__ax, [rel sdistr_Uniform]
	push	__ax
%else
	push	ptr_t sdistr_Uniform
%endif
	push	__cx
	call	memcpy wrt ..plt
	add		__sp, sizeof(size_t) + 2 * sizeof(ptr_t)

	; restore __cx
	pop		__cx

	; fix the fld instructions operands
	mov		[__cx + sdistr_Uniform.postrange0 - sdistr_Uniform - sizeof(ptr_t)], __di
	add		__di, sizeof(float)
	mov		[__cx + sdistr_Uniform.postrange1 - sdistr_Uniform - sizeof(ptr_t)], __di
	sub		__di, sizeof(float)

	; advance the pointers
	add		__cx, sdistr_Uniform_size
	add		__di, sdistr_UniformData_size
	ret

; ========================================
; Scalar Bicubic Interpolation
; ========================================

sdistr_BicubicInterp:
	; TODO
	fld1
.end:
sdistr_BicubicInterp_size		equ	(sdistr_BicubicInterp.end - sdistr_BicubicInterp)
sdistr_BicubicInterpData_size	equ	0

sdistr_BicubicInterpCompile:
	; TODO
	ret

; ========================================
; Vector Constant
; ========================================

vdistr_Constant:
	movups	xmm5, [dummy_addr]	; val; this address is filled upon compilation
.end:
vdistr_Constant_size		equ	(vdistr_Constant.end - vdistr_Constant)
vdistr_ConstantData_size	equ	(4 * sizeof(float))

vdistr_ConstantCompile:
	; save __cx, memcpy resets it
	push	__cx

	; first, memcpy data to data buffer
	push	size_t 3 * sizeof(float)
	lea		__dx, [__si + ptcVDistr_Constant.Val]
	push	__dx
	push	__di
	call	memcpy wrt ..plt
	add		__sp, sizeof(size_t) + 2 * sizeof(ptr_t)

	; restore __cx but also keep a copy on the stack
	mov		__cx, [__sp]

	; now, memcpy the code
	push	size_t vdistr_Constant_size
%ifidn __BITS__,64
	lea		__ax, [rel vdistr_Constant]
	push	__ax
%else
	push	ptr_t vdistr_Constant
%endif
	push	__cx
	call	memcpy wrt ..plt
	add		__sp, sizeof(size_t) + 2 * sizeof(ptr_t)

	; restore __cx
	pop		__cx

	; fix the movups instruction operand
	mov		[__cx + vdistr_Constant.end - vdistr_Constant - sizeof(ptr_t)], __di

	; advance the pointers
	add		__cx, vdistr_Constant_size
	add		__di, vdistr_ConstantData_size
	ret

; ========================================
; Vector Uniform
; ========================================

vdistr_Uniform:
	movups	xmm6, [dummy_addr]	; range[0]; this address is filled upon compilation
.postrange0:
	movups	xmm7, [dummy_addr]	; range[1]; this address is filled upon compilation
.postrange1:
	subps	xmm7, xmm6
	; save off __ax and __dx
	push	__ax
	push	__dx
	; get 3 random numbers and store them on the stack, leave one float of slack
	; (oh, the rhyme!)
	sub		__sp, 4 * sizeof(float)
	frand 2 * sizeof(ptr_t) + 4 * sizeof(float)
	fstp	dword [__sp]
	fwait
	frand 2 * sizeof(ptr_t) + 4 * sizeof(float)
	fstp	dword [__sp + 4]
	fwait
	frand 2 * sizeof(ptr_t) + 4 * sizeof(float)
	fstp	dword [__sp + 8]
	fwait
	; load them into xmm
	movups	xmm5, [__sp]
	; pop the temp variables off the stack
	add		__sp, 4 * sizeof(float)
	; restore __ax and __dx
	pop		__dx
	pop		__ax
	mulps	xmm5, xmm7
	addps	xmm5, xmm6
.end:
vdistr_Uniform_size		equ	(vdistr_Uniform.end - vdistr_Uniform)
vdistr_UniformData_size	equ	(4 * 2 * sizeof(float))

vdistr_UniformCompile:
	; save __cx, memcpy resets it
	push	__cx

	; first, memcpy data to data buffer
	; have to do this twice since we have two vectors: range[0]...
	push	size_t 3 * sizeof(float)
	lea		__dx, [__si + ptcVDistr_Uniform.Ranges]
	push	__dx
	push	__di
	call	memcpy wrt ..plt
	add		__sp, 3 * sizeof(float)
	; ...and range[1]
	push	size_t 3 * sizeof(float)
	lea		__dx, [__si + ptcVDistr_Uniform.Ranges + 3 * sizeof(float)]
	push	__dx
	; copy 16 bytes after the first one - source has 3 components, but SSE
	; registers are 4-component anyway, we'll take those 4 bytes of slack...
	lea		__dx, [__di + 4 * sizeof(float)]
	push	__dx
	call	memcpy wrt ..plt
	add		__sp, sizeof(size_t) + 2 * sizeof(ptr_t)

	; restore __cx but also keep a copy on the stack
	mov		__cx, ptr_t [__sp]

	; now, memcpy the code
	push	size_t vdistr_Uniform_size
%ifidn __BITS__,64
	lea		__ax, [rel vdistr_Uniform]
	push	__ax
%else
	push	ptr_t vdistr_Uniform
%endif
	push	__cx
	call	memcpy wrt ..plt
	add		__sp, sizeof(size_t) + 2 * sizeof(ptr_t)

	; restore __cx
	pop		__cx

	; fix the movups instructions operands
	mov		[__cx + vdistr_Uniform.postrange0 - vdistr_Uniform - sizeof(ptr_t)], __di
	add		__di, 4 * sizeof(float)
	mov		[__cx + vdistr_Uniform.postrange1 - vdistr_Uniform - sizeof(ptr_t)], __di
	sub		__di, 4 * sizeof(float)

	; advance the pointers
	add		__cx, vdistr_Uniform_size
	add		__di, vdistr_UniformData_size
	ret

; ========================================
; Vector Bicubic Interpolation
; ========================================

vdistr_BicubicInterp:
	; TODO
.end:
vdistr_BicubicInterp_size		equ	(vdistr_BicubicInterp.end - vdistr_BicubicInterp)
vdistr_BicubicInterpData_size	equ	0

vdistr_BicubicInterpCompile:
	; TODO
	ret

; ========================================
; Flag-based colour component mixing
; ========================================

%macro cdistr_mix 0
	; simple case - both RGB and alpha are to be written
	test	__ax, (ptcCF_SetRGB | ptcCF_SetAlpha)
	; parity bit holds information about equality
	jnp		%%rgb
	movaps	xmm5, xmm6
	jmp		.end
%%rgb:
	test	__ax, ptcCF_SetRGB
	jz		%%alpha
	; mask out alpha from the distribution value and add in the original alpha
	mov		__ax, [__sp + 3 * sizeof(float)]
	movups	xmm7, [__ax]
	andps	xmm6, xmm7
	andnps	xmm7, xmm5
	addps	xmm6, xmm7
	movaps	xmm5, xmm6
	jmp		.end
%%alpha:
	test	__ax, ptcCF_SetAlpha
	jz		.end
	; mask out RGB from the distribution value and add in the original RGB
	mov		__ax, [__sp + 4 * sizeof(float)]
	movups	xmm7, [__ax]
	andps	xmm6, xmm7
	andnps	xmm7, xmm5
	addps	xmm6, xmm7
	movaps	xmm5, xmm6
%endmacro

; ========================================
; Colour Constant
; ========================================

cdistr_Constant:
	movups	xmm6, [dummy_addr]	; val; this address is filled upon compilation
.postval:
	cdistr_mix
.end:
cdistr_Constant_size		equ	(cdistr_Constant.end - cdistr_Constant)
cdistr_ConstantData_size	equ	(4 * sizeof(float))

cdistr_ConstantCompile:
	; save off __cx
	push	__cx

	; first, memcpy data to data buffer
	push	size_t cdistr_ConstantData_size
	lea		__dx, [__si + ptcCDistr_Constant.Val]
	push	__dx
	push	__di
	call	memcpy wrt ..plt
	add		__sp, sizeof(size_t) + 2 * sizeof(ptr_t)

	; restore __cx but also keep a copy on the stack
	mov		__cx, ptr_t [__sp]

	; now, memcpy the code
	push	size_t cdistr_Constant_size
%ifidn __BITS__,64
	lea		__ax, [rel cdistr_Constant]
	push	__ax
%else
	push	ptr_t cdistr_Constant
%endif
	push	__cx
	call	memcpy wrt ..plt
	add		__sp, sizeof(size_t) + 2 * sizeof(ptr_t)

	; restore __cx
	pop		__cx

	; fix the movups instruction operand
	mov		[__cx + cdistr_Constant.postval - cdistr_Constant - sizeof(ptr_t)], __di

	; advance the pointers
	add		__cx, cdistr_Constant_size
	add		__di, cdistr_ConstantData_size
	ret


; ========================================
; Colour Uniform
; ========================================

cdistr_Uniform:
	movups	xmm6, [dummy_addr]	; range[0]; this address is filled upon compilation
.postrange0:
	movups	xmm7, [dummy_addr]	; range[1]; this address is filled upon compilation
.postrange1:
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
	frand 2 * 4 * sizeof(float) + 2 * sizeof(ptr_t)
	fstp	dword [__sp + 4 * sizeof(float)]
	fwait
	frand 2 * 4 * sizeof(float) + 2 * sizeof(ptr_t)
	fstp	dword [__sp + 5 * sizeof(float)]
	fwait
	frand 2 * 4 * sizeof(float) + 2 * sizeof(ptr_t)
	fstp	dword [__sp + 6 * sizeof(float)]
	fwait
	frand 2 * 4 * sizeof(float) + 2 * sizeof(ptr_t)
	fstp	dword [__sp + 7 * sizeof(float)]
	fwait
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
.end:
cdistr_Uniform_size		equ	(cdistr_Uniform.end - cdistr_Uniform)
cdistr_UniformData_size	equ	(2 * 4 * sizeof(float))

cdistr_UniformCompile:
	; save off __cx
	push	__cx

	; first, memcpy data to data buffer
	push	size_t cdistr_UniformData_size
	lea		__dx, [__si + ptcVDistr_Constant.Val]
	push	__dx
	push	__di
	call	memcpy wrt ..plt
	add		__sp, sizeof(size_t) + 2 * sizeof(ptr_t)

	; restore __cx but also keep a copy on the stack
	mov		__cx, ptr_t [__sp]

	; now, memcpy the code
	push	dword cdistr_Uniform_size
%ifidn __BITS__,64
	lea		__ax, [rel cdistr_Uniform]
	push	__ax
%else
	push	ptr_t cdistr_Uniform
%endif
	push	__cx
	call	memcpy wrt ..plt
	add		__sp, sizeof(size_t) + 2 * sizeof(ptr_t)

	; restore __cx
	pop		__cx

	; fix the movups instructions operands
	mov		[__cx + (cdistr_Uniform.postrange0 - cdistr_Uniform - sizeof(ptr_t))], __di
	add		__di, 4 * sizeof(float)
	mov		[__cx + (cdistr_Uniform.postrange1 - cdistr_Uniform - sizeof(ptr_t))], __di
	sub		__di, 4 * sizeof(float)

	; advance the pointers
	add		__cx, cdistr_Uniform_size
	add		__di, cdistr_UniformData_size
	ret

; ========================================
; Colour Bicubic Interpolation
; ========================================

cdistr_BicubicInterp:
	; TODO
	nop
.end:
cdistr_BicubicInterp_size		equ	(cdistr_BicubicInterp.end - cdistr_BicubicInterp)
cdistr_BicubicInterpData_size	equ	0

cdistr_BicubicInterpCompile:
	; TODO
	ret

; ========================================
; Compilation helper macros
; ========================================

; macro to calculate the size of a single distribution instance
; %1: register containing the distribution ID
; %2: register where the code size will be written
; %3: register where the data size will be written
; %4; data type prefix (s, v or c)
%macro distr_size 4
	cmp		%1, ptcDID_Constant
	jne		%%uniform
	mov		%2, %4 %+ distr_Constant_size
	mov		%3, %4 %+ distr_ConstantData_size
	jmp		%%end
%%uniform:
	cmp		%1, ptcDID_Uniform
	jne		%%bicubic
	mov		%2, %4 %+ distr_Uniform_size
	mov		%3, %4 %+ distr_UniformData_size
	jmp		%%end
%%bicubic:
	cmp		%1, ptcDID_BicubicInterp
	jne		%%end
	mov		%2, %4 %+ distr_BicubicInterp_size
	mov		%3, %4 %+ distr_BicubicInterpData_size
%%end:
%endmacro

; a macro that chooses the right compile procedure
; %1: register containing the distribution ID
; %2; data type prefix (s, v or c)
%macro distr_compile 2
	cmp		%1, ptcDID_Constant
	jne		%%uniform
	call	%2 %+ distr_ConstantCompile
	jmp		%%end
%%uniform:
	cmp		%1, ptcDID_Uniform
	jne		%%bicubic
	call	%2 %+ distr_UniformCompile
	jmp		%%end
%%bicubic:
	cmp		%1, ptcDID_BicubicInterp
	jne		%%end
	call	%2 %+ distr_BicubicInterpCompile
%%end:
%endmacro


%endif ; PTC_DISTRIBUTIONS
