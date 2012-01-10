; particlasm main module
; Copyright (C) 2011, Leszek Godlewski <lg@inequation.org>

bits 32

; put EVERYTHING in the code section
section .text

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
extern malloc
extern free

; entry points to the library
global ptcCompileEmitter:function
global ptcProcessEmitter:function
global ptcReleaseEmitter:function

%include "ptc_distributions.inc"
%include "ptc_modules.inc"

; macro to accumulate buffer length by module
%macro AccumModBuf 1
	cmp		edx, ptcMID_ %+ %1
	jne		%%skip
	call	mod_ %+ %1 %+ Measure
	mov		eax, [spawnbuf_len]
	add		eax, ebx
	mov		[spawnbuf_len], eax
	mov		eax, [procbuf_len]
	add		eax, ecx
	mov		[procbuf_len], eax
	mov		eax, [databuf_len]
	add		eax, edx
	mov		[databuf_len], eax
	jmp		.next
%%skip:
%endmacro

; macro to compile module code into the given buffer pointer
%macro CompileMod 1
	cmp		edx, ptcMID_ %+ %1
	jne		%%skip
	call	mod_ %+ %1 %+ Compile
	jne		.compile_next
%%skip:
%endmacro

; emitter compilation
ptcCompileEmitter:
	%push		ptcCompileEmitterContext
	%stacksize	flat
	%assign		%$localsize 0
	%local		spawnbuf_len:dword	; particle spawn code buffer size
	%local		procbuf_len:dword	; particle processing code buffer size
	%local		databuf_len:dword	; particle data buffer size
	%local		spawnbuf_ptr:dword	; particle spawn code buffer pointer
	%local		procbuf_ptr:dword	; particle processing code buffer pointer
	%local		databuf_ptr:dword	; particle data buffer pointer
	%arg		emitter:dword

	enter   %$localsize, 0

	push	ebx
	push	esi
	push	edi

	; calculate the sizes of the buffers
	; include additional space for a ret instruction at the end of spawn code
	mov		dword [spawnbuf_len], (mod_SimulatePostCompile.postret - mod_SimulatePostCompile.preret)
	mov		dword [procbuf_len], 0
	mov		dword [databuf_len], 0
	mov		esi, [emitter]
	mov		esi, [esi + ptcEmitter.Head]
.buf_loop:
	cmp		esi, 0
	je		.alloc
	mov		edx, [esi + ptcModuleHeader.ModuleID]

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
	mov		esi, [esi + ptcModuleHeader.Next]
	jmp		.buf_loop

.alloc:
	; also add the simulation module
	mov		ecx, [procbuf_len]
	add		ecx, (mod_SimulatePre_size + mod_SimulatePost_size)
	mov		[procbuf_len], ecx

	; allocate the buffers
	; FIXME: error checking!
	mov		eax, [spawnbuf_len]
	push	eax
	call	malloc
	mov		[spawnbuf_ptr], eax
	mov		eax, [procbuf_len]
	push	eax
	call	malloc
	mov		[procbuf_ptr], eax
	mov		eax, [databuf_len]
	push	eax
	call	malloc
	mov		[databuf_ptr], eax
	; pop the pointers off the stack
	add		esp, 4 * 3

	; compile
	; place the pointers on the stack in the convention that modules expect them
	push	eax
	mov		eax, [procbuf_ptr]
	push	eax
	mov		eax, [spawnbuf_ptr]
	push	eax

	; add in the simulation preprocessing module
	call	mod_SimulatePreCompile

	; start traversing the list
	mov		esi, [emitter]
	mov		esi, [esi + ptcEmitter.Head]
.compile_loop:
	cmp		esi, 0
	je		.done
	mov		edx, [esi + ptcModuleHeader.ModuleID]

	; detect modules and copy in their code and data
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
	mov		esi, [esi + ptcModuleHeader.Next]
	jmp		.compile_loop

.done:
	; add in the simulation postprocessing module
	call	mod_SimulatePostCompile

	; pop all the pointers off the stack
	add		esp, 4 * 4

	; write the pointers to the emitter struct
	mov		esi, [emitter]
	mov		edx, [databuf_ptr]
	mov		[esi + ptcEmitter.InternalPtr1], edx
	mov		edx, [procbuf_ptr]
	mov		[esi + ptcEmitter.InternalPtr2], edx
	mov		edx, [spawnbuf_ptr]
	mov		[esi + ptcEmitter.InternalPtr3], edx

	pop		edi
	pop		esi
	pop		ebx

	; return success
	mov     eax, 1

	leave

	ret
	%pop

ptcProcessEmitter:
	xor     eax, eax
	ret

ptcReleaseEmitter:
	%push		ptcReleaseEmitterContext
	%stacksize	flat
	%assign		%$localsize 0
	%arg		emitter:dword

	enter   %$localsize, 0

	; free the buffers
	mov		esi, [emitter]
	mov		eax, [esi + ptcEmitter.InternalPtr1]
	push	eax
	call	free
	mov		eax, [esi + ptcEmitter.InternalPtr2]
	push	eax
	call	free
	mov		eax, [esi + ptcEmitter.InternalPtr3]
	push	eax
	call	free

	; pop all the pointers off the stack
	add		esp, 4 * 3

	leave
	ret
	%pop
