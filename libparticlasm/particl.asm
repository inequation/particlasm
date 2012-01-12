; particlasm main module
; Copyright (C) 2011, Leszek Godlewski <lg@inequation.org>

; we're targeting a 32-bit Pentium M
bits 32
cpu p3

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
global ptcInternalSpawnParticles:function
global ptcInternalProcessParticles:function
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

; loads particle data into registers; assumes pointer in esi
%macro load_particle 0
	mov		edx, [esi + ptcParticle.Active]
	fld		dword [esi + ptcParticle.Size]
	fld		dword [esi + ptcParticle.Rotation]
	fld		dword [esi + ptcParticle.Time]
	fld		dword [esi + ptcParticle.TimeScale]
	movups	xmm1, [esi + ptcParticle.Colour]
	movups	xmm2, [esi + ptcParticle.Location]
	movups	xmm3, [esi + ptcParticle.Velocity]
	movups	xmm4, [esi + ptcParticle.Accel]
%endmacro

; stores particle data into memory; assumes pointer in esi
%macro store_particle 0
	movups	[esi + ptcParticle.Accel], xmm4
	movups	[esi + ptcParticle.Velocity], xmm3
	movups	[esi + ptcParticle.Location], xmm2
	movups	[esi + ptcParticle.Colour], xmm1
	fstp	dword [esi + ptcParticle.TimeScale]
	fstp	dword [esi + ptcParticle.Time]
	fstp	dword [esi + ptcParticle.Rotation]
	fstp	dword [esi + ptcParticle.Size]
	mov		[esi + ptcParticle.Active], edx
%endmacro

; macro that pushes the time step into the correct registers
%macro push_step 1
	fld		dword %1
	; load it into the low order part of the register
	movss	xmm0, %1
	; and broadcast it to the entire one
	shufps	xmm0, xmm0, 00h
%endmacro

; macro that pops the time step off the registers
%macro pop_step 0
	fsubp	st0, st0
%endmacro

; macro that advances and wraps the particle pointer around buffer size
%macro advance_particle_ptr 0
	add		esi, ptcParticle_size
	mov		eax, [ebx + ptcEmitter.ParticleBuf]
	mov		edx, [ebx + ptcEmitter.MaxParticles]
	add		eax, edx
	cmp		esi, eax
	jl		%%skip
	mov		eax, ptcParticle_size
	mul		edx
	sub		esi, eax
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
	mov		dword [spawnbuf_len], 16
	mov		dword [procbuf_len], 16
	mov		dword [databuf_len], 16
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

; arguments begin at esp+8
ptcInternalSpawnParticles:
	%push		ptcInternalSpawnParticlesContext
	%stacksize	flat
	%assign		%$localsize 0
	%arg		emitter:dword
	%arg		step:dword
	%arg		count:dword

	enter	%$localsize, 0

	push	ebx
	push	esi
	push	edi

	;get_GOT

	; initialize the FPU to make sure the stack is clear and there are no
	; exceptions
	;finit
	;fwait

	; start at a random index to reduce chances of a collision
	call	rand
	mov		ebx, [emitter]
	xor		edx, edx
	mov		ecx, [ebx + ptcEmitter.MaxParticles]
	div		ecx
	mov		eax, ptcParticle_size
	mul		edx
	mov		esi, eax

	mov		ecx, [count]
	add		esi, [ebx + ptcEmitter.ParticleBuf]

	; kick off the loop - find a free spot in the buffer
.find_spot:
	mov		edx, [esi + ptcParticle.Active]
	test	edx, edx
	jz		.cont
	advance_particle_ptr
	jmp		.find_spot
.cont:
	; increase particle counter
	mov		edx, [ebx + ptcEmitter.NumParticles]
	inc		edx;
	mov		[ebx + ptcEmitter.NumParticles], edx
	; clear particle data
	mov		[esi + ptcParticle.Active], dword 1
	; calculate time scale: 1 / (LifeTimeFixed + frand() * LifeTimeRandom)
	fld1
	fld		dword [ebx + ptcEmitter.LifeTimeFixed]
	fld		dword [ebx + ptcEmitter.LifeTimeRandom]
	frand
	; st0=frand(), st1=LTR, st2=LTF, st3=1.0
	fmulp	st1, st0
	; st0=frand()*LTR, st1=LTF, st2=1.0
	faddp	st1, st0
	; st0=LTF+frand()*LTR, st1=1.0
	fdivp	st1, st0
	; st0=1.0/t
	fstp	dword [esi + ptcParticle.TimeScale]
	; sync
	fwait
	mov		[esi + ptcParticle.Time], dword 0
	fld1
	fstp	dword [esi + ptcParticle.Colour]
	fwait
	; reuse the 1.0 float we've just stored
	mov		eax, dword [esi + ptcParticle.Colour]
	mov		dword [esi + (ptcParticle.Colour + 4)], eax
	mov		dword [esi + (ptcParticle.Colour + 8)], eax
	mov		dword [esi + (ptcParticle.Colour + 12)], eax
	mov		[esi + ptcParticle.Location], dword 0
	mov		[esi + (ptcParticle.Location + 4)], dword 0
	mov		[esi + (ptcParticle.Location + 8)], dword 0
	mov		[esi + (ptcParticle.Location + 12)], dword 0
	mov		[esi + ptcParticle.Rotation], dword 0
	mov		dword [esi + ptcParticle.Size], eax
	mov		[esi + ptcParticle.Velocity], dword 0
	mov		[esi + (ptcParticle.Velocity + 4)], dword 0
	mov		[esi + (ptcParticle.Velocity + 8)], dword 0
	mov		[esi + (ptcParticle.Velocity + 12)], dword 0
	mov		[esi + ptcParticle.Accel], dword 0
	mov		[esi + (ptcParticle.Accel + 4)], dword 0
	mov		[esi + (ptcParticle.Accel + 8)], dword 0
	mov		[esi + (ptcParticle.Accel + 12)], dword 0
	; save off ecx and ebx
	push	ebx
	push	ecx
	; load particle data into registers
	load_particle
	push_step [step]
	; call the spawn code
	mov		ebx, [emitter]
	call	[ebx + ptcEmitter.InternalPtr2]
	pop_step
	; store new particle state
	store_particle
	; restore ecx and ebx
	pop		ecx
	pop		ebx
	add		esi, ptcParticle_size

	advance_particle_ptr
	dec		ecx
	jmp		.find_spot

	pop		edi
	pop		esi
	pop		ebx

	leave
	ret
	%pop

ptcInternalProcessParticles:
	%push		ptcInternalProcessParticlesContext
	%stacksize	flat
	%assign		%$localsize 0
	%arg		emitter:dword
	%arg		step:dword
	%arg		cameraCS:dword
	%arg		buffer:dword
	%arg		maxVertices:dword

	enter   %$localsize, 0

	leave
	ret
	%pop

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
