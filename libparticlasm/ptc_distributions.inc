; particlasm value distributions code
; Copyright (C) 2011, Leszek Godlewski <lg@inequation.org>

%ifndef PTC_DISTRIBUTIONS
%define PTC_DISTRIBUTIONS
; declarations
%include "libparticlasm.inc"

; libc rand()
extern rand
; equal to 1.f / RAND_MAX (as per the stdlib.h declaration)
INV_RAND_MAX:	dd	4.65661287524579692411e-10

; scalar distributions put their result into st0
; vector and colour - into xmm5

; utility macro that places a random number in the [0..1] range into st0
%macro frand 0
	call rand
	; temp variable - the fild instruction needs to read from memory
	push eax
	fild dword [esp]
	; load in the divisor
	mov eax, dword INV_RAND_MAX
	push eax
	fild dword [esp]
	; pop the two dwords off the stack
	add esp, 8
	; rand() * (1.f / RAND_MAX)
	fmulp st1, st0
%endmacro

sdistr_Constant:
	fld dword [0]	; val; this address is filled upon compilation
.end:
sdistr_Constant_size	equ	(sdistr_Constant.end - sdistr_Constant + 1)

sdistr_Uniform:
	fld dword [0]	; range[0]; this address is filled upon compilation
	fld dword [0]	; range[1]; this address is filled upon compilation
	; range[1] - range[0]
	fsub st1, st0
	frand
	; multiply the range span by the random number
	fmulp st2, st0
	; add the lower bound
	faddp st1, st0
.end:

sdistr_Uniform_size	equ	(sdistr_Uniform.end - sdistr_Uniform + 1)

sdistr_BicubicInterp:
	; TODO
	fld1
.end:
sdistr_BicubicInterp_size	equ	(sdistr_BicubicInterp.end - sdistr_BicubicInterp + 1)

vdistr_Constant:
	movups xmm5, [0]	; val; this address is filled upon compilation
.end:
vdistr_Constant_size	equ	(vdistr_Constant.end - vdistr_Constant + 1)

vdistr_Uniform:
	movups xmm6, [0]	; range[0]; this address is filled upon compilation
	movups xmm7, [0]	; range[1]; this address is filled upon compilation
	subps xmm7, xmm6
	; get 3 random numbers
	frand
	frand
	frand
	; store them on the stack, leave one float of slack (oh, the rhyme!)
	sub esp, 4 * 4
	mov eax, esp
	fst dword [eax]
	add eax, 4
	fst dword [eax]
	add eax, 4
	fst dword [eax]
	; load them into xmm
	movups xmm5, [esp]
	; pop the temp variables off the stack
	add esp, 4 * 4
	mulps xmm5, xmm7
	addps xmm5, xmm6
.end:
vdistr_Uniform_size	equ	(vdistr_Uniform.end - vdistr_Uniform + 1)

vdistr_BicubicInterp:
	; TODO
	nop
.end:
vdistr_BicubicInterp_size	equ	(vdistr_BicubicInterp.end - vdistr_BicubicInterp + 1)

cdistr_Constant:
	movups xmm5, [0]	; val; this address is filled upon compilation
.end:
cdistr_Constant_size	equ	(cdistr_Constant.end - cdistr_Constant + 1)

cdistr_Uniform:
	movups xmm6, [0]	; range[0]; this address is filled upon compilation
	movups xmm7, [0]	; range[1]; this address is filled upon compilation
	subps xmm7, xmm6
	; get 4 random numbers
	frand
	frand
	frand
	frand
	; store them on the stack, leave one float of slack (oh, the rhyme!)
	sub esp, 4 * 4
	mov eax, esp
	fst dword [eax]
	add eax, 4
	fst dword [eax]
	add eax, 4
	fst dword [eax]
	add eax, 4
	fst dword [eax]
	; load them into xmm
	movups xmm5, [esp]
	; pop the temp variables off the stack
	add esp, 4 * 4
	mulps xmm5, xmm7
	addps xmm5, xmm6
.end:
cdistr_Uniform_size	equ	(cdistr_Uniform.end - cdistr_Uniform + 1)

cdistr_BicubicInterp:
	; TODO
	nop
.end:
cdistr_BicubicInterp_size	equ	(cdistr_BicubicInterp.end - cdistr_BicubicInterp + 1)

; FIXME: automate this to detect the largest code blocks
sdistr_maxsize	equ	sdistr_Uniform_size
vdistr_maxsize	equ	vdistr_Uniform_size
cdistr_maxsize	equ	cdistr_Uniform_size

%endif ; PTC_DISTRIBUTIONS
