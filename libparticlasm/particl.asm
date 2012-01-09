; particlasm main module
; Copyright (C) 2011, Leszek Godlewski <lg@inequation.org>

bits 32

; declarations
%include "libparticlasm.inc"

; declare the GOT symbol
extern _GLOBAL_OFFSET_TABLE_
; macro used to store the _GLOBAL_OFFSET_TABLE_ address in ebx
%macro get_GOT 0
	call	%%getgot
%%getgot:
	pop		ebx
	add		ebx, _GLOBAL_OFFSET_TABLE_+$$-%%getgot wrt ..gotpc
%endmacro

; some useful cstdlib functions
extern printf
extern malloc
extern free
extern memcpy

; entry points to the library
global ptcCompileEmitter:function
global ptcProcessEmitter:function
global ptcReleaseEmitter:function

%include "ptc_distributions.inc"
%include "ptc_modules.inc"

; simulation module, preprocessing
mod_SimulatePre:
	; if the particle is not active, just skip it
	test	edx, edx
	jz		.end
	; T += t * Ts
	fld		st0
	fmul	st0, st2
	faddp	st3, st0
	; kill the particle if we're past its lifetime
	fld1
	fcomip	st3
	jle		.end
	mov		edx, 0h
	mov		eax, [ebx + ptcEmitter.NumParticles]
	dec		eax
	mov		[ebx + ptcEmitter.NumParticles], eax
.end:
mod_SimulatePre_size	equ	(mod_SimulatePre.end - mod_SimulatePre)

; simulation module, postprocessing
mod_SimulatePost:
	; v += a * t
	movups	xmm7, xmm3
	mulps	xmm7, xmm4
	addps	xmm2, xmm7
	; x += v * t
	movups	xmm7, xmm2
	mulps	xmm7, xmm4
.end:
	addps	xmm1, xmm7
mod_SimulatePost_size	equ	(mod_SimulatePost.end - mod_SimulatePost + 1)

; macro to accumulate buffer length by module
%macro AccumModBuf 1
	cmp		edx, ptcMID_ %+ %1
	jne		%%skip
	call	mod_ %+ %1 %+ Measure
	; we don't have a spare register to work with - put eax away for a sec...
	push	eax
	mov		eax, [spawnbuf]
	add		eax, ebx
	mov		[spawnbuf], eax
	; ...and take it back
	pop		eax
	mov		ebx, [procbuf]
	add		ebx, ecx
	mov		[procbuf], ebx
	mov		ebx, [databuf]
	add		ebx, edx
	mov		[databuf], ebx
	jne		.next
%%skip:
%endmacro

; macro to compile module code into the given buffer pointer
%macro CompileMod 1
	cmp		edx, ptcMID_ %+ %1
	jne		%%skip
	jne		.compile_next
%%skip:
%endmacro

; emitter compilation
ptcCompileEmitter:
	%push		ptcCompileEmitterContext
	%stacksize	flat
	%assign		%$localsize 0
	%local		spawnbuf:dword	; particle spawn code buffer size
	%local		procbuf:dword	; particle processing code buffer size
	%local		databuf:dword	; particle data buffer size
	%arg		emitter:dword

	enter   %$localsize, 0

	; calculate the sizes of the buffers
	mov		dword [spawnbuf], 0
	mov		dword [procbuf], 0
	mov		dword [databuf], 0
	mov		eax, [emitter]
	mov		eax, [eax + ptcEmitter.Head]
.buf_loop:
	cmp		eax, 0h
	je		.alloc
	mov		edx, [eax + ptcModuleHeader.ModuleID]

	; detect modules and add in their sizes
	AccumModBuf InitialLocation
	AccumModBuf InitialRotation
	AccumModBuf InitialSize
	AccumModBuf InitialVelocity
	AccumModBuf InitialColour
	AccumModBuf Velocity
	AccumModBuf Acceleration
	AccumModBuf Colour
	AccumModBuf Size
	AccumModBuf Gravity
.next:
	; get next module pointer
	mov		eax, [eax + ptcEmitter.Head]
	jmp		.buf_loop

.alloc:
	; also add the simulation module
	mov		ecx, [procbuf]
	add		ecx, mod_SimulatePre_size
	add		ecx, mod_SimulatePost_size
	mov		[procbuf], ecx

	; allocate the buffers
	push	ecx
	call	malloc

	; compile
	xor		ecx, ecx
	mov		eax, [emitter + ptcEmitter.Head]
.compile_loop:
	cmp		eax, 0h
	je		.alloc
	mov		edx, [eax]

	; detect modules and add in their sizes
	CompileMod InitialLocation
	CompileMod InitialRotation
	CompileMod InitialSize
	CompileMod InitialVelocity
	CompileMod InitialColour
	CompileMod Velocity
	CompileMod Acceleration
	CompileMod Colour
	CompileMod Size
	CompileMod Gravity
.compile_next:
	; get next module pointer
	mov		eax, [eax + ptcEmitter.Head]
	jmp		.compile_loop

	push	eax
	call	free

	mov     eax, 0h

	leave

	ret
	%pop

ptcProcessEmitter:
		mov     eax, 0h
        ret

ptcReleaseEmitter:
		mov		eax, 0h
        ret

