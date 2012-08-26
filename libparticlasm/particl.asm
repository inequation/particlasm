; particlasm main module
; Copyright (C) 2011-2012, Leszek Godlewski <lg@inequation.org>

; architecture-specific defines
%ifidni PTC_ARCH,X64
	cpu X64

	; x64 doesn't have pushad/popad, so make a macro for this
	%macro pushad 0
		push		rax
		push		rcx
		push		rdx
		push		rbx
		push		rsp
		push		rbp
		push		rsi
		push		rdi
	%endmacro
	%macro popad 0
		pop			rdi
		pop			rsi
		pop			rbp
		pop			rsp
		pop			rbx
		pop			rdx
		pop			rcx
		pop			rax
	%endmacro

	; data type macros - quad word
	%define resp	resq
	%define ressize_t	resq
	%define ptr		qword
	%define size_t	qword

	; libc call wrapper
	; %1 - function name
	%macro libc_call 1
		call	%1 wrt ..plt
	%endmacro
%else
	cpu P3

	; register aliases for pushing/popping
	%define rax		eax
	%define rbx		ebx
	%define rcx		ecx
	%define rdx		edx
	%define rsi		esi
	%define rdi		edi
	%define rsp		esp

	; data type macros - double word
	%define resp	resd
	%define ressize_t	resd
	%define ptr		dword
	%define size_t	dword

	; libc call wrapper
	%macro libc_call 1
		call	%1
	%endmacro
%endif
; floats are always 32-bit
%define float		dword

; by default, all addresses are absolute
default abs

; declarations
%include "libparticlasm.inc"

; libc import
extern rand

; declare the GOT symbol
extern _GLOBAL_OFFSET_TABLE_
; macro used to store the _GLOBAL_OFFSET_TABLE_ address in ebx
%macro get_GOT 0
	call	%%getgot
%%getgot:
	pop		rbx
	add		rbx, _GLOBAL_OFFSET_TABLE_+$$-%%getgot wrt ..gotpc
%endmacro

; entry points to the library
global ptcInternalMeasureModule:function
global ptcInternalCompileModule:function
global ptcInternalSpawnParticles:function
global ptcInternalProcessParticles:function

; equal to 1.f / RAND_MAX (as per the stdlib.h declaration) in hex representation
%define INV_RAND_MAX	0x30000000	;4.65661287524579692411e-10

; utility macro that places a random number in the [0..1] range in st0
; %1 - additional stack offset, 0 by default
%macro frand 0-1 0
	; need to make the call a far one since the code will live in a totally
	; different memory block
	call	[rsp + (4 + %1)]
	; temp variable - the fild instruction needs to read from memory
	push	rax
	fild	float [rsp]
	add		rsp, 4
	; load in the inverse RAND_MAX
	fld		float [rsp + (8 + %1)]
	; pop the rand() result off the stack
	; rand() * (1.f / RAND_MAX)
	fmulp	st1, st0
%endmacro

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
	mov		rax, [spawnbuf_len]
	add		rax, rbx
	mov		[spawnbuf_len], rax
	mov		rax, [procbuf_len]
	add		rax, rcx
	mov		[procbuf_len], rax
	mov		rax, [databuf_len]
	add		rax, rdx
	mov		[databuf_len], rax
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

; loads particle data into registers; assumes pointer in rsi
%macro load_particle 0
	mov		edx, [rsi + ptcParticle.Active]
	fld		float [rsi + ptcParticle.Size]
	fld		float [rsi + ptcParticle.Rotation]
	fld		float [rsi + ptcParticle.Time]
	fld		float [rsi + ptcParticle.TimeScale]
	movups	xmm1, [rsi + ptcParticle.Colour]
	movups	xmm2, [rsi + ptcParticle.Location]
	movups	xmm3, [rsi + ptcParticle.Velocity]
	movups	xmm4, [rsi + ptcParticle.Accel]
%endmacro

; stores particle data into memory; assumes pointer in esi
%macro store_particle 0
	; our vectors are 12 bytes wide, so these stores require some SSE magic:
	; first store the low order half of the register, then broadcast the 3rd
	; float onto the entire register and store it as a scalar at an offset
	movlps	[rsi + ptcParticle.Accel], xmm4
	shufps	xmm4, xmm4, 0xAA
	movss	[rsi + ptcParticle.Accel + 8], xmm4
	movlps	[rsi + ptcParticle.Velocity], xmm3
	shufps	xmm3, xmm3, 0xAA
	movss	[rsi + ptcParticle.Velocity + 8], xmm3
	movlps	[rsi + ptcParticle.Location], xmm2
	shufps	xmm2, xmm2, 0xAA
	movss	[rsi + ptcParticle.Location + 8], xmm2
	; colour is 16 bytes wide, no magic required
	movups	[rsi + ptcParticle.Colour], xmm1
	fstp	float [rsi + ptcParticle.TimeScale]
	fstp	float [rsi + ptcParticle.Time]
	fstp	float [rsi + ptcParticle.Rotation]
	fstp	float [rsi + ptcParticle.Size]
	mov		[rsi + ptcParticle.Active], edx
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
	add		rsi, ptcParticle_size
	mov		rdx, [rbx + ptcEmitter.MaxParticles]
	mov		rax, ptcParticle_size
	mul		rdx
	mov		rdx, [rbx + ptcEmitter.ParticleBuf]
	add		rax, rdx
	cmp		rsi, rax
	jl		%%skip
	mov		rsi, rdx
%%skip:
%endmacro

ptcInternalMeasureModule:
	%push		ptcInternalMeasureModuleContext
	%stacksize	flat
	%assign		%$localsize 0
	%arg		module:ptr
	%arg		spawnCodeBufLenPtr:ptr
	%arg		processCodeBufLenPtr:ptr
	%arg		dataBufLenPtr:ptr
	%local		spawnbuf_len:size_t	; particle spawn code buffer size
	%local		procbuf_len:size_t	; pointer particle processing code buffer size
	%local		databuf_len:size_t	; pointer particle data buffer size

	enter   %$localsize, 0

	; save off working registers
	pushad

	; initialize counters
	mov		rsi, [spawnCodeBufLenPtr]
	mov		rdx, [rsi]
	mov		[spawnbuf_len], rdx
	mov		rsi, [processCodeBufLenPtr]
	mov		rdx, [rsi]
	mov		[procbuf_len], rdx
	mov		rsi, [dataBufLenPtr]
	mov		rdx, [esi]
	mov		[databuf_len], rdx

	mov		rsi, [module]

	; if module == NULL, add in the simulation module
	test	rsi, rsi
	jnz		.normal

	mov		rax, [spawnbuf_len]
	inc		rax	; add in 1 byte for a ret instruction
	mov		[spawnbuf_len], rax
	mov		rax, [procbuf_len]
	add		rax, mod_SimulatePre_size + mod_SimulatePost_size
	mov		[procbuf_len], rax
	jmp		.end

.normal:
	mov		rdx, [rsi + ptcModuleHeader.ModuleID]

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
	mov		rdx, [spawnbuf_len]
	mov		rdi, [spawnCodeBufLenPtr]
	mov		[rdi], rdx
	mov		rdx, [procbuf_len]
	mov		rdi, [processCodeBufLenPtr]
	mov		[rdi], rdx
	mov		rdx, [databuf_len]
	mov		rdi, [dataBufLenPtr]
	mov		[rdi], rdx

	; restore working registers
	popad

	leave
	ret
	%pop

ptcInternalCompileModule:
	%push		ptcCompileEmitterContext
	%stacksize	flat
	%assign		%$localsize 0
	%arg		module:ptr
	%arg		spawnCodeBufPtr:ptr
	%arg		processCodeBufPtr:ptr
	%arg		dataBufPtr:ptr

	enter   %$localsize, 0

	; save off working registers
	push	rbx
	push	rsi
	push	rdi

	; place the pointers on the stack in the convention that modules expect them
	mov		rax, [dataBufPtr]
	mov		rax, [rax]
	push	rax
	mov		rax, [processCodeBufPtr]
	mov		rax, [rax]
	push	rax
	mov		rax, [spawnCodeBufPtr]
	mov		rax, [rax]
	push	rax
	mov		rsi, [module]

	; check for simulation pre- and post-processing modules
	cmp		rsi, ptr -1
	jne		.try_post
	; add in the simulation preprocessing module
	call	mod_SimulatePreCompile
	jmp		.end
.try_post:
	cmp		rsi, ptr -2
	jne		.normal
	; add in the simulation postprocessing module
	call	mod_SimulatePostCompile
	jmp		.end
.normal:
	mov		rdx, [rsi + ptcModuleHeader.ModuleID]

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
	pop		rdx
	mov		rax, [spawnCodeBufPtr]
	mov		[rax], rdx
	pop		rdx
	mov		rax, [processCodeBufPtr]
	mov		[rax], rdx
	pop		rdx
	mov		rax, [dataBufPtr]
	mov		[rax], rdx

	; restore working registers
	pop		rdi
	pop		rsi
	pop		rbx

	leave
	ret
	%pop

ptcInternalSpawnParticles:
	%push		ptcInternalSpawnParticlesContext
	%stacksize	flat
	%assign		%$localsize 0
	%arg		emitter:ptr
	%arg		step:float
	%arg		count:size_t

	enter	%$localsize, 0

	push	rbx
	push	rsi
	push	rdi

	; initialize the FPU to make sure the stack is clear and there are no
	; exceptions
	finit
	fwait

	; start at a random index to reduce chances of a collision
	libc_call rand
	mov		rbx, [emitter]
	xor		rdx, rdx
	mov		rcx, [rbx + ptcEmitter.MaxParticles]
	div		rcx
	mov		rax, ptcParticle_size
	mul		rdx
	mov		rsi, rax

	mov		rcx, [count]
	add		rsi, [rbx + ptcEmitter.ParticleBuf]

	; kick off the loop - find a free spot in the buffer
.find_spot:
	mov		edx, [rsi + ptcParticle.Active]
	test	edx, edx	; intentionally always 32-bit
	jz		.cont
	advance_particle_ptr
	jmp		.find_spot
.cont:
	; increase particle counter
	mov		rdx, [rbx + ptcEmitter.NumParticles]
	inc		rdx;
	mov		[rbx + ptcEmitter.NumParticles], rdx
	; clear particle data
	mov		[rsi + ptcParticle.Active], dword 1
	; calculate time scale: 1 / (LifeTimeFixed + frand() * LifeTimeRandom)
	fld1
	fld		float [rbx + ptcEmitterConfig.LifeTimeFixed]
	fld		float [rbx + ptcEmitterConfig.LifeTimeRandom]
	; save off rcx and rdx
	push	rcx
	push	rdx
	; need the function address and inverse RAND_MAX on the stack
	push	dword INV_RAND_MAX
%ifidni PTC_ARCH,X64
	push	ptr [rel rand wrt ..plt]
%else
	push	rand
%endif
	push	0	; dummy value
	frand
	add		rsp, 4 * 3
	; restore ecx and edx
	pop		rdx
	pop		rcx
	; st0=frand(), st1=LTR, st2=LTF, st3=1.0
	fmulp	st1, st0
	; st0=frand()*LTR, st1=LTF, st2=1.0
	faddp	st1, st0
	; st0=LTF+frand()*LTR, st1=1.0
	fdivp	st1, st0
	; st0=1.0/t
	fstp	float [rsi + ptcParticle.TimeScale]
	; sync
	fwait
	mov		[rsi + ptcParticle.Time], float 0
	fld1
	fstp	float [rsi + ptcParticle.Colour]
	fwait
	; reuse the 1.0 float we've just stored
	mov		eax, float [rsi + ptcParticle.Colour]
	mov		float [esi + (ptcParticle.Colour + 4)], eax
	mov		float [esi + (ptcParticle.Colour + 8)], eax
	mov		float [esi + (ptcParticle.Colour + 12)], eax
	mov		[rsi + ptcParticle.Location], float 0
	mov		[rsi + (ptcParticle.Location + 4)], float 0
	mov		[rsi + (ptcParticle.Location + 8)], float 0
	mov		[rsi + ptcParticle.Rotation], float 0
	mov		float [rsi + ptcParticle.Size], eax
	mov		[rsi + ptcParticle.Velocity], float 0
	mov		[rsi + (ptcParticle.Velocity + 4)], float 0
	mov		[rsi + (ptcParticle.Velocity + 8)], float 0
	mov		[rsi + ptcParticle.Accel], float 0
	mov		[rsi + (ptcParticle.Accel + 4)], float 0
	mov		[rsi + (ptcParticle.Accel + 8)], float 0
	; save off working registers
	push	rbx
	push	rcx
	push	rsi
	; load particle data into registers
	load_particle
	push_step [step]
	; load the pointers that we need
%ifidni PTC_ARCH,X64
	push	ptr [rel MASK_ALPHA]
	push	ptr [rel MASK_RGB]
%else
	push	MASK_ALPHA
	push	MASK_RGB
%endif
	push	dword INV_RAND_MAX
%ifidni PTC_ARCH,X64
	push	ptr [rel rand wrt ..plt]
%else
	push	rand
%endif
	mov		rbx, [emitter]

	; call the spawn code
	call	[rbx + ptcEmitter.InternalPtr2]

	; mark as active
	mov		edx, dword 1

	; restore previous state
	add		rsp, 4 * 4
	pop_step
	; store new particle state
	store_particle
	; restore working registers
	pop		rsi
	pop		rcx
	pop		rbx

	advance_particle_ptr

	dec		rcx
	jecxz	.end
	jmp		.find_spot

.end:
	pop		rdi
	pop		rsi
	pop		rbx

	leave
	ret
	%pop

ptcInternalProcessParticles:
	%push		ptcInternalProcessParticlesContext
	%stacksize	flat
	%assign		%$localsize 0
	%arg		emitter:ptr
	%arg		startPtr:ptr
	%arg		endPtr:ptr
	%arg		step:float
	%arg		cameraCS:ptr
	%arg		buffer:ptr
	%arg		maxVertices:size_t
	%local		verts:ptr

	enter   %$localsize, 0

	push	rbx
	push	rsi
	push	rdi

	mov		rbx, [maxVertices]
	mov		[verts], rbx

	; initialize pointers
	mov		rsi, [startPtr]
	mov		rdi, [endPtr]
	mov		rbx, [emitter]

	; kick off the loop
.loop:
	; save off working registers
	push	rbx
	push	rcx
	push	rsi
	push	rdi
	; load particle data into registers
	load_particle
	push_step [step]
	; load the pointers that we need
%ifidni PTC_ARCH,X64
	push	ptr [rel MASK_ALPHA]
	push	ptr [rel MASK_RGB]
%else
	push	MASK_ALPHA
	push	MASK_RGB
%endif
	push	dword INV_RAND_MAX
%ifidni PTC_ARCH,X64
	push	ptr [rel rand wrt ..plt]
%else
	push	rand
%endif
	mov		rbx, [emitter]

	; call the processing code
	call	[rbx + ptcEmitter.InternalPtr3]

	; restore previous state
	add		rsp, 4 * 4
	pop_step
	; store new particle state
	store_particle
	; restore working registers
	pop		rdi
	pop		rsi
	pop		rcx
	pop		rbx

	; skip vertex emitting if there's nothing to emit it for
	test	edx, edx
	jz		.cont

	push	rdi

	; emit vertices
	mov		rdx, [maxVertices]
	test	rdx, rdx
	jz		.end
	mov		rdi, [buffer]
	; just copy the colour into an xmm
	movups	xmm7, [rsi + ptcParticle.Colour]
	; find the vertices of the particle
	movups	xmm0, [rsi + ptcParticle.Location]
	; load in the particle size and broadcast it to all components
	movss	xmm3, [rsi + ptcParticle.Size]
	shufps	xmm3, xmm3, 0x00
	; load in 0.5 and broadcast it to all components
	push	float 0x3F000000
	movss	xmm4, [rsp]
	add		rsp, 4
	shufps	xmm4, xmm4, 0x00
	; load in the camera coordinate system
	push	rsi
	mov		rsi, [cameraCS]
	movups	xmm1, [rsi + 3 * 4]
	movups	xmm2, [rsi + 6 * 4]
	pop		rsi
	; multiply by particle size and 0.5
	mulps	xmm1, xmm3
	mulps	xmm1, xmm4
	mulps	xmm2, xmm3
	mulps	xmm2, xmm4
	; at this point we have:
	; xmm0 - particle location (centre)
	; xmm1 - right camera vector * particle size * 0.5
	; xmm2 - up camera vector * particle size * 0.5
	; use these to calc the different vertices into xmm3
	; vertex 1 = location + right + up
	movaps	xmm3, xmm0
	addps	xmm3, xmm1
	addps	xmm3, xmm2
	; vertex 2 = location - right + up
	movaps	xmm4, xmm0
	subps	xmm4, xmm1
	addps	xmm4, xmm2
	; vertex 3 = location - right - up
	movaps	xmm5, xmm0
	subps	xmm5, xmm1
	subps	xmm5, xmm2
	; vertex 4 = location + right - up
	movaps	xmm6, xmm0
	addps	xmm6, xmm1
	subps	xmm6, xmm2

	; save vertex data
	movups	[rdi + ptcVertex_size * 0 + ptcVertex.Colour], xmm7
	movlps	[rdi + ptcVertex_size * 0 + ptcVertex.Location], xmm3
	shufps	xmm3, xmm3, 0xAA
	movss	[rdi + ptcVertex_size * 0 + ptcVertex.Location + 8], xmm3
	mov		[rdi + ptcVertex_size * 0 + ptcVertex.TexCoords], word 1
	mov		[rdi + ptcVertex_size * 0 + ptcVertex.TexCoords + 2], word 0

	movups	[rdi + ptcVertex_size * 1 + ptcVertex.Colour], xmm7
	movlps	[rdi + ptcVertex_size * 1 + ptcVertex.Location], xmm4
	shufps	xmm4, xmm4, 0xAA
	movss	[rdi + ptcVertex_size * 1 + ptcVertex.Location + 8], xmm4
	mov		[rdi + ptcVertex_size * 1 + ptcVertex.TexCoords], word 0
	mov		[rdi + ptcVertex_size * 1 + ptcVertex.TexCoords + 2], word 0

	movups	[rdi + ptcVertex_size * 2 + ptcVertex.Colour], xmm7
	movlps	[rdi + ptcVertex_size * 2 + ptcVertex.Location], xmm5
	shufps	xmm5, xmm5, 0xAA
	movss	[rdi + ptcVertex_size * 2 + ptcVertex.Location + 8], xmm5
	mov		[rdi + ptcVertex_size * 2 + ptcVertex.TexCoords], word 0
	mov		[rdi + ptcVertex_size * 2 + ptcVertex.TexCoords + 2], word 1

	movups	[rdi + ptcVertex_size * 3 + ptcVertex.Colour], xmm7
	movlps	[rdi + ptcVertex_size * 3 + ptcVertex.Location], xmm6
	shufps	xmm6, xmm6, 0xAA
	movss	[rdi + ptcVertex_size * 3 + ptcVertex.Location + 8], xmm6
	mov		[rdi + ptcVertex_size * 3 + ptcVertex.TexCoords], word 1
	mov		[rdi + ptcVertex_size * 3 + ptcVertex.TexCoords + 2], word 1

	; advance pointers
	sub		rdx, 4
	mov		[maxVertices], rdx
	add		rdi, ptcVertex_size * 4
	mov		[buffer], rdi
	pop		rdi
.cont:
	add		rsi, ptcParticle_size
	cmp		rsi, rdi
	jl		.loop

.end:
	mov		rbx, [maxVertices]
	mov		rax, [verts]
	sub		rax, rbx

	pop		rdi
	pop		rbx

	leave
	ret
	%pop
