; particlasm value distributions code
; Copyright (C) 2011-2012, Leszek Godlewski <lg@inequation.org>

%ifndef PTC_DISTRIBUTIONS
%define PTC_DISTRIBUTIONS
; declarations
%include "libparticlasm.inc"

; libc memcpy()
extern memcpy

; scalar distributions put their result into st0
; vector and colour - into xmm5
; NOTE: colour distributions expect input colour in xmm5 and flags in eax up
; front!

; Compilation calling convention:
; ecx - pointer to working code buffer
; edi - pointer to working data buffer

; Other than that, same calling conventions apply for spawning and processing as
; for modules.

; utility macro that places a random number in the [0..1] range in st0
; %1 - additional stack offset
%macro frand 1
	; need to make the call a far one since the code will live in a totally
	; different memory block
	call	[esp + (4 + %1)]
	; temp variable - the fild instruction needs to read from memory
	push	eax
	fild	dword [esp]
	add		esp, 4
	; load in the inverse RAND_MAX
	fld		dword [esp + (8 + %1)]
	; pop the rand() result off the stack
	; rand() * (1.f / RAND_MAX)
	fmulp	st1, st0
%endmacro

; ========================================
; Scalar Constant
; ========================================

sdistr_Constant:
	fld		dword [0xFFFFFFFF]	; val; this address is filled upon compilation
.end:
sdistr_Constant_size		equ	(sdistr_Constant.end - sdistr_Constant)
sdistr_ConstantData_size	equ	4

sdistr_ConstantCompile:
	; first, copy data to data buffer
	mov		edx, [esi + ptcSDistr_Constant.Val]
	mov		[edi], edx

	; save off ecx
	push	ecx

	; now, memcpy the code
	push	dword sdistr_Constant_size
	push	dword sdistr_Constant
	push	ecx
	call	memcpy
	add		esp, 4 * 3

	; restore ecx
	pop		ecx

	; fix the fld instruction operand
	mov		[ecx + sdistr_Constant.end - sdistr_Constant - 4], edi

	; advance the pointers
	add		ecx, sdistr_Constant_size
	add		edi, sdistr_ConstantData_size
	ret

; ========================================
; Scalar Uniform
; ========================================

sdistr_Uniform:
	fld		dword [0xFFFFFFFF]	; range[0]; this address is filled upon compilation
.postrange0:
	fld		dword [0xFFFFFFFF]	; range[1]; this address is filled upon compilation
.postrange1:
	; exchange the registers - FPU is stupid...
	fxch	st1
	; save off eax and edx
	push	eax
	push	edx
	; temp variable
	sub		esp, 4
	; st1=range[1], st0=range[0]
	; range[1] - range[0]
	fsub	st1, st0
	; save range[0] to temp var
	fstp	dword [esp]
	; st0=range[1] - range[0]
	fwait
	frand	12
	; multiply the range span by the random number
	; st0=frand(), st1=range[1] - range[0]
	fmulp	st1, st0
	; add the lower bound
	; st0=frand() * (range[1] - range[0])
	fld		dword [esp]
	faddp	st1, st0
	add		esp, 4
	; restore eax and edx
	pop		edx
	pop		eax
	; st0=distribution result
.end:
sdistr_Uniform_size		equ	(sdistr_Uniform.end - sdistr_Uniform)
sdistr_UniformData_size	equ	8

sdistr_UniformCompile:
	; first, copy data to data buffer
	mov		edx, [esi + ptcSDistr_Uniform.Range]
	mov		[edi], edx
	mov		edx, [esi + ptcSDistr_Uniform.Range + 4]
	mov		[edi + 4], edx

	; save off ecx
	push	ecx

	; now, memcpy the code
	push	dword sdistr_Uniform_size
	push	dword sdistr_Uniform
	push	ecx
	call	memcpy
	add		esp, 4 * 3

	; restore ecx
	pop		ecx

	; fix the fld instructions operands
	mov		[ecx + sdistr_Uniform.postrange0 - sdistr_Uniform - 4], edi
	add		edi, 4
	mov		[ecx + sdistr_Uniform.postrange1 - sdistr_Uniform - 4], edi
	sub		edi, 4

	; advance the pointers
	add		ecx, sdistr_Uniform_size
	add		edi, sdistr_UniformData_size
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
	movups	xmm5, [0xFFFFFFFF]	; val; this address is filled upon compilation
.end:
vdistr_Constant_size		equ	(vdistr_Constant.end - vdistr_Constant)
vdistr_ConstantData_size	equ	(4 * 4)

vdistr_ConstantCompile:
	; save ecx, memcpy resets it
	push	ecx

	; first, memcpy data to data buffer
	push	dword 4 * 3
	lea		edx, [esi + ptcVDistr_Constant.Val]
	push	edx
	push	edi
	call	memcpy
	add		esp, 4 * 3

	; restore ecx but also keep a copy on the stack
	mov		ecx, dword [esp]

	; now, memcpy the code
	push	dword vdistr_Constant_size
	push	dword vdistr_Constant
	push	ecx
	call	memcpy
	add		esp, 4 * 3

	; restore ecx
	pop		ecx

	; fix the movups instruction operand
	mov		[ecx + vdistr_Constant.end - vdistr_Constant - 4], edi

	; advance the pointers
	add		ecx, vdistr_Constant_size
	add		edi, vdistr_ConstantData_size
	ret

; ========================================
; Vector Uniform
; ========================================

vdistr_Uniform:
	movups	xmm6, [0xFFFFFFFF]	; range[0]; this address is filled upon compilation
.postrange0:
	movups	xmm7, [0xFFFFFFFF]	; range[1]; this address is filled upon compilation
.postrange1:
	subps	xmm7, xmm6
	; save off eax and edx
	push	eax
	push	edx
	; get 3 random numbers and store them on the stack, leave one float of slack
	; (oh, the rhyme!)
	sub		esp, 4 * 4
	frand 4 * 6
	fstp	dword [esp]
	fwait
	frand 4 * 6
	fstp	dword [esp + 4]
	fwait
	frand 4 * 6
	fstp	dword [esp + 8]
	fwait
	; load them into xmm
	movups	xmm5, [esp]
	; pop the temp variables off the stack
	add		esp, 4 * 4
	; restore eax and edx
	pop		edx
	pop		eax
	mulps	xmm5, xmm7
	addps	xmm5, xmm6
.end:
vdistr_Uniform_size		equ	(vdistr_Uniform.end - vdistr_Uniform)
vdistr_UniformData_size	equ	(4 * 4 * 2)

vdistr_UniformCompile:
	; save ecx, memcpy resets it
	push	ecx

	; first, memcpy data to data buffer
	; have to do this twice since we have two vectors: range[0]...
	push	dword 4 * 3
	lea		edx, [esi + ptcVDistr_Uniform.Ranges]
	push	edx
	push	edi
	call	memcpy
	add		esp, 4 * 3
	; ...and range[1]
	push	dword 4 * 3
	lea		edx, [esi + ptcVDistr_Uniform.Ranges + 4 * 3]
	push	edx
	; copy 16 bytes after the first one - source has 3 components, but SSE
	; registers are 4-component anyway, we'll take those 4 bytes of slack...
	lea		edx, [edi + 4 * 4]
	push	edx
	call	memcpy
	add		esp, 4 * 3

	; restore ecx but also keep a copy on the stack
	mov		ecx, dword [esp]

	; now, memcpy the code
	push	dword vdistr_Uniform_size
	push	dword vdistr_Uniform
	push	ecx
	call	memcpy
	add		esp, 4 * 3

	; restore ecx
	pop		ecx

	; fix the movups instructions operands
	mov		[ecx + vdistr_Uniform.postrange0 - vdistr_Uniform - 4], edi
	add		edi, 4 * 4
	mov		[ecx + vdistr_Uniform.postrange1 - vdistr_Uniform - 4], edi
	sub		edi, 4 * 4

	; advance the pointers
	add		ecx, vdistr_Uniform_size
	add		edi, vdistr_UniformData_size
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
	test	eax, (ptcCF_SetRGB | ptcCF_SetAlpha)
	; parity bit holds information about equality
	jnp		%%rgb
	movaps	xmm5, xmm6
	jmp		.end
%%rgb:
	test	eax, ptcCF_SetRGB
	jz		%%alpha
	; mask out alpha from the distribution value and add in the original alpha
	mov		eax, [esp + 12]
	movups	xmm7, [eax]
	andps	xmm6, xmm7
	andnps	xmm7, xmm5
	addps	xmm6, xmm7
	movaps	xmm5, xmm6
	jmp		.end
%%alpha:
	test	eax, ptcCF_SetAlpha
	jz		.end
	; mask out RGB from the distribution value and add in the original RGB
	mov		eax, [esp + 16]
	movups	xmm7, [eax]
	andps	xmm6, xmm7
	andnps	xmm7, xmm5
	addps	xmm6, xmm7
	movaps	xmm5, xmm6
%endmacro

; ========================================
; Colour Constant
; ========================================

cdistr_Constant:
	movups	xmm6, [0xFFFFFFFF]	; val; this address is filled upon compilation
.postval:
	cdistr_mix
.end:
cdistr_Constant_size		equ	(cdistr_Constant.end - cdistr_Constant)
cdistr_ConstantData_size	equ	(4 * 4)

cdistr_ConstantCompile:
	; save off ecx
	push	ecx

	; first, memcpy data to data buffer
	push	dword cdistr_ConstantData_size
	lea		edx, [esi + ptcCDistr_Constant.Val]
	push	edx
	push	edi
	call	memcpy
	add		esp, 4 * 3

	; restore ecx but also keep a copy on the stack
	mov		ecx, dword [esp]

	; now, memcpy the code
	push	dword cdistr_Constant_size
	push	dword cdistr_Constant
	push	ecx
	call	memcpy
	add		esp, 4 * 3

	; restore ecx
	pop		ecx

	; fix the movups instruction operand
	mov		[ecx + cdistr_Constant.postval - cdistr_Constant - 4], edi

	; advance the pointers
	add		ecx, cdistr_Constant_size
	add		edi, cdistr_ConstantData_size
	ret


; ========================================
; Colour Uniform
; ========================================

cdistr_Uniform:
	movups	xmm6, [0xFFFFFFFF]	; range[0]; this address is filled upon compilation
.postrange0:
	movups	xmm7, [0xFFFFFFFF]	; range[1]; this address is filled upon compilation
.postrange1:
	subps	xmm7, xmm6
	; save off eax and edx
	push	eax
	push	edx
	; reserve some space on the stack - we need to lay off the original colour
	; as well as random floats
	sub		esp, (4 * 4 * 2)
	; save off the original colour
	movups	[esp], xmm5
	; get 4 random numbers and store them on the stack, leave one float of slack
	; (oh, the rhyme!)
	frand 4 * 4 * 2 + 4 * 2
	fstp	dword [esp + 16]
	fwait
	frand 4 * 4 * 2 + 4 * 2
	fstp	dword [esp + 20]
	fwait
	frand 4 * 4 * 2 + 4 * 2
	fstp	dword [esp + 24]
	fwait
	frand 4 * 4 * 2 + 4 * 2
	fstp	dword [esp + 28]
	fwait
	; load the random numbers into xmm
	movups	xmm5, [esp + 16]
	; get distribution value
	mulps	xmm5, xmm7
	addps	xmm6, xmm5
	; restore original colour in xmm5
	movups	xmm5, [esp]
	; pop the temp variables off the stack
	add		esp, (4 * 4 * 2)
	; restore eax and edx
	pop		edx
	pop		eax
	; mix the colours
	cdistr_mix
.end:
cdistr_Uniform_size		equ	(cdistr_Uniform.end - cdistr_Uniform)
cdistr_UniformData_size	equ	(4 * 4 * 2)

cdistr_UniformCompile:
	; save off ecx
	push	ecx

	; first, memcpy data to data buffer
	push	dword cdistr_UniformData_size
	lea		edx, [esi + ptcVDistr_Constant.Val]
	push	edx
	push	edi
	call	memcpy
	add		esp, 4 * 3

	; restore ecx but also keep a copy on the stack
	mov		ecx, dword [esp]

	; now, memcpy the code
	push	dword cdistr_Uniform_size
	push	dword cdistr_Uniform
	push	ecx
	call	memcpy
	add		esp, 4 * 3

	; restore ecx
	pop		ecx

	; fix the movups instructions operands
	mov		[ecx + (cdistr_Uniform.postrange0 - cdistr_Uniform - 4)], edi
	add		edi, 4 * 4
	mov		[ecx + (cdistr_Uniform.postrange1 - cdistr_Uniform - 4)], edi
	sub		edi, 4 * 4

	; advance the pointers
	add		ecx, cdistr_Uniform_size
	add		edi, cdistr_UniformData_size
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
