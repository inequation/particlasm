; particlasm main module
; Copyright (C) 2011, Leszek Godlewski <lg@inequation.org>

; declarations
%include "libparticlasm.inc"

; declare the GOT symbol
extern _GLOBAL_OFFSET_TABLE_
; some useful cstdlib functions
extern printf
extern malloc
extern free
extern memcpy

; entry points to the library
global ptcCompileEmitter:function
global ptcProcessEmitter:function
global ptcReleaseEmitter:function

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
	nop
mod_SimulatePre_size	equ	(mod_SimulatePre.end - mod_SimulatePre + 1)

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
	mov		ecx, [spawnbuf]
	add		ecx, mod_ %+ %1 %+ Spawn_size
	mov		[spawnbuf], ecx
	mov		ecx, [procbuf]
	add		ecx, mod_ %+ %1 %+ Process_size
	mov		[spawnbuf], ecx
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
	%stacksize	small
	%assign		%$localsize 0
	%arg		emitter:dword
	%local		spawnbuf:dword	; particle spawn code buffer size
	%local		procbuf:dword		; particle processing code buffer size

	;push    ebp
	;mov     ebp, esp
	enter   %$localsize,0

	push    ebx
	call    .get_GOT
.get_GOT:
	pop     ebx
	add     ebx,_GLOBAL_OFFSET_TABLE_+$$-.get_GOT wrt ..gotpc

	; calculate the sizes of the buffers
	mov		dword [spawnbuf], 0
	mov		dword [procbuf], 0
	mov		eax, [emitter + ptcEmitter.Head]
.buf_loop:
	cmp		eax, 0h
	je		.alloc
	mov		edx, [eax]

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
	mov		[spawnbuf], ecx

	; allocate the buffer
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

	;mov     ebx, [ebp-4]
	;mov     esp, ebp
	;pop     ebp
	ret
	%pop

ptcProcessEmitter:	push    ebp
        mov     ebp, esp
        push    ebx
        call    .get_GOT
.get_GOT:
        pop     ebx
        add     ebx,_GLOBAL_OFFSET_TABLE_+$$-.get_GOT wrt ..gotpc

        mov     eax, 0h

        mov     ebx, [ebp-4]
        mov     esp, ebp
        pop     ebp
        ret

ptcReleaseEmitter:	push    ebp
        mov     ebp, esp
        push    ebx
        call    .get_GOT
.get_GOT:
        pop     ebx
        add     ebx,_GLOBAL_OFFSET_TABLE_+$$-.get_GOT wrt ..gotpc

        mov     ebx, [ebp-4]
        mov     esp, ebp
        pop     ebp
        ret

