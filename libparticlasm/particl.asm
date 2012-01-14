; particlasm main module
; Copyright (C) 2011, Leszek Godlewski <lg@inequation.org>

; we're targeting a 32-bit Pentium M
bits 32
cpu p3

; put EVERYTHING in the code section
section .text

; declarations
%include "libparticlasm.inc"

extern rand

; declare the GOT symbol
extern _GLOBAL_OFFSET_TABLE_
; macro used to store the _GLOBAL_OFFSET_TABLE_ address in ebx
%macro get_GOT 0
	call	%%getgot
%%getgot:
	pop		ebx
	add		ebx, _GLOBAL_OFFSET_TABLE_+$$-%%getgot wrt ..gotpc
%endmacro

; entry points to the library
global ptcInternalMeasureModule:function
global ptcInternalCompileModule:function
global ptcInternalSpawnParticles:function
global ptcInternalProcessParticles:function

; equal to 1.f / RAND_MAX (as per the stdlib.h declaration) in hex representation
%define INV_RAND_MAX	0x30000000	;4.65661287524579692411e-10

MASK_RGB		dd	0xFFFFFFFF
				dd	0xFFFFFFFF
				dd	0xFFFFFFFF
				dd	0x00000000
MASK_ALPHA		dd	0x00000000
				dd	0x00000000
				dd	0x00000000
				dd	0xFFFFFFFF

%include "ptc_distributions.inc"
%include "ptc_modules.inc"

; macro to measure module buffer lengths
%macro MeasureModBuf 1
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
	jmp		.end
%%skip:
%endmacro

; macro to compile module code into the given buffer pointer
%macro CompileMod 1
	cmp		edx, ptcMID_ %+ %1
	jne		%%skip
	call	mod_ %+ %1 %+ Compile
	jne		.end
%%skip:
%endmacro

; loads particle data into registers; assumes pointer in esi
%macro load_particle 0
	mov		edx, [esi + ptcParticle.Active]
	fld		dword [esi + ptcParticle.Size]
	fld		dword [esi + ptcParticle.Rotation]
	fld		dword [esi + ptcParticle.Time]
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
	fstp	st0
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

ptcInternalMeasureModule:
	%push		ptcInternalMeasureModuleContext
	%stacksize	flat
	%assign		%$localsize 0
	%arg		module:dword
	%arg		spawnCodeBufLenPtr:dword
	%arg		processCodeBufLenPtr:dword
	%arg		dataBufLenPtr:dword
	%local		spawnbuf_len:dword	; particle spawn code buffer size
	%local		procbuf_len:dword	; pointer particle processing code buffer size
	%local		databuf_len:dword	; pointer particle data buffer size

	enter   %$localsize, 0

	; save off working registers
	pusha

	; initialize counters
	mov		esi, [spawnCodeBufLenPtr]
	mov		edx, [esi]
	mov		[spawnbuf_len], edx
	mov		esi, [processCodeBufLenPtr]
	mov		edx, [esi]
	mov		[procbuf_len], edx
	mov		esi, [dataBufLenPtr]
	mov		edx, [esi]
	mov		[databuf_len], edx

	mov		esi, [module]

	; if module == NULL, add in the simulation module
	test	esi, esi
	jnz		.normal

	mov		eax, [spawnbuf_len]
	inc		eax	; keep 1 byte for a ret instruction
	mov		[spawnbuf_len], eax
	mov		eax, [procbuf_len]
	add		eax, mod_SimulatePre_size + mod_SimulatePost_size
	mov		[procbuf_len], eax
	jmp		.end

.normal:
	mov		edx, [esi + ptcModuleHeader.ModuleID]

	; detect modules and add in their sizes
	MeasureModBuf InitialLocation
	MeasureModBuf InitialRotation
	MeasureModBuf InitialSize
	MeasureModBuf InitialVelocity
	MeasureModBuf InitialColour
	MeasureModBuf Velocity
	MeasureModBuf Acceleration
	MeasureModBuf Colour
	MeasureModBuf Size
	MeasureModBuf Gravity

.end:
	; copy measured length to the provided pointers
	mov		edx, [spawnbuf_len]
	mov		edi, [spawnCodeBufLenPtr]
	mov		[edi], edx
	mov		edx, [procbuf_len]
	mov		edi, [processCodeBufLenPtr]
	mov		[edi], edx
	mov		edx, [databuf_len]
	mov		edi, [dataBufLenPtr]
	mov		[edi], edx

	; restore working registers
	popa

	leave
	ret
	%pop

ptcInternalCompileModule:
	%push		ptcCompileEmitterContext
	%stacksize	flat
	%assign		%$localsize 0
	%arg		module:dword
	%arg		spawnCodeBufPtr:dword
	%arg		processCodeBufPtr:dword
	%arg		dataBufPtr:dword

	enter   %$localsize, 0

	; save off working registers
	push	ebx
	push	esi
	push	edi

	; place the pointers on the stack in the convention that modules expect them
	mov		eax, [dataBufPtr]
	mov		eax, [eax]
	push	eax
	mov		eax, [processCodeBufPtr]
	mov		eax, [eax]
	push	eax
	mov		eax, [spawnCodeBufPtr]
	mov		eax, [eax]
	push	eax
	mov		esi, [module]

	; check for simulation pre- and post-processing modules
	cmp		esi, dword -1
	jne		.try_post
	; add in the simulation preprocessing module
	call	mod_SimulatePreCompile
	jmp		.end
.try_post:
	cmp		esi, dword -2
	jne		.normal
	; add in the simulation postprocessing module
	call	mod_SimulatePostCompile
	jmp		.end
.normal:
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

.end:
	; return pointers after advancing
	pop		edx
	mov		eax, [spawnCodeBufPtr]
	mov		[eax], edx
	pop		edx
	mov		eax, [processCodeBufPtr]
	mov		[eax], edx
	pop		edx
	mov		eax, [dataBufPtr]
	mov		[eax], edx

	; restore working registers
	pop		edi
	pop		esi
	pop		ebx

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
	finit
	fwait

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
	; need the function address and inverse RAND_MAX on the stack
	push	dword INV_RAND_MAX
	push	rand
	push	0	; dummy value
	frand	0
	add		esp, 4 * 2
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
	; load the pointers that we need
	push	MASK_ALPHA
	push	MASK_RGB
	push	dword INV_RAND_MAX
	push	rand
	mov		ebx, [emitter]

	; call the spawn code
	call	[ebx + ptcEmitter.InternalPtr2]

	; restore previous state
	add		esp, 3 * 4
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
