; particlasm main module
; Copyright (C) 2011-2012, Leszek Godlewski <lg@inequation.org>

cpu PTC_ARCH

; architecture-specific defines
%ifidni PTC_ARCH,X64
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

	; pointer type macro - quad word
	%define resp	resq
%else
	; register aliases for pushing/popping
	%define rax		eax
	%define rbx		ebx
	%define rcx		ecx
	%define rdx		edx
	%define rsi		esi
	%define rdi		edi

	; pointer type macro - double word
	%define resp	resd
%endif

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
	fld		dword [esi + ptcParticle.TimeScale]
	movups	xmm1, [esi + ptcParticle.Colour]
	movups	xmm2, [esi + ptcParticle.Location]
	movups	xmm3, [esi + ptcParticle.Velocity]
	movups	xmm4, [esi + ptcParticle.Accel]
%endmacro

; stores particle data into memory; assumes pointer in esi
%macro store_particle 0
	; our vectors are 12 bytes wide, so these stores require some SSE magic:
	; first store the low order half of the register, then broadcast the 3rd
	; float onto the entire register and store it as a scalar at an offset
	movlps	[esi + ptcParticle.Accel], xmm4
	shufps	xmm4, xmm4, 0xAA
	movss	[esi + ptcParticle.Accel + 8], xmm4
	movlps	[esi + ptcParticle.Velocity], xmm3
	shufps	xmm3, xmm3, 0xAA
	movss	[esi + ptcParticle.Velocity + 8], xmm3
	movlps	[esi + ptcParticle.Location], xmm2
	shufps	xmm2, xmm2, 0xAA
	movss	[esi + ptcParticle.Location + 8], xmm2
	; colour is 16 bytes wide, no magic required
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
	fstp	st0
%endmacro

; macro that advances and wraps the particle pointer around buffer size
%macro advance_particle_ptr 0
	add		esi, ptcParticle_size
	mov		edx, [ebx + ptcEmitter.MaxParticles]
	mov		eax, ptcParticle_size
	mul		edx
	mov		edx, [ebx + ptcEmitter.ParticleBuf]
	add		eax, edx
	cmp		esi, eax
	jl		%%skip
	mov		esi, edx
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
	pushad

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
	popad

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
	push	rbx
	push	rsi
	push	rdi

	; place the pointers on the stack in the convention that modules expect them
	mov		eax, [dataBufPtr]
	mov		eax, [eax]
	push	rax
	mov		eax, [processCodeBufPtr]
	mov		eax, [eax]
	push	rax
	mov		eax, [spawnCodeBufPtr]
	mov		eax, [eax]
	push	rax
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
	pop		rdx
	mov		eax, [spawnCodeBufPtr]
	mov		[eax], edx
	pop		rdx
	mov		eax, [processCodeBufPtr]
	mov		[eax], edx
	pop		rdx
	mov		eax, [dataBufPtr]
	mov		[eax], edx

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
	%arg		emitter:dword
	%arg		step:dword
	%arg		count:dword

	enter	%$localsize, 0

	push	rbx
	push	rsi
	push	rdi

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
	fld		dword [ebx + ptcEmitterConfig.LifeTimeFixed]
	fld		dword [ebx + ptcEmitterConfig.LifeTimeRandom]
	; save off ecx and edx
	push	rcx
	push	rdx
	; need the function address and inverse RAND_MAX on the stack
	push	dword INV_RAND_MAX
	push	rand
	push	0	; dummy value
	frand	0
	add		esp, 4 * 3
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
	mov		[esi + ptcParticle.Rotation], dword 0
	mov		dword [esi + ptcParticle.Size], eax
	mov		[esi + ptcParticle.Velocity], dword 0
	mov		[esi + (ptcParticle.Velocity + 4)], dword 0
	mov		[esi + (ptcParticle.Velocity + 8)], dword 0
	mov		[esi + ptcParticle.Accel], dword 0
	mov		[esi + (ptcParticle.Accel + 4)], dword 0
	mov		[esi + (ptcParticle.Accel + 8)], dword 0
	; save off working registers
	push	rbx
	push	rcx
	push	rsi
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

	; mark as active
	mov		edx, dword 1

	; restore previous state
	add		esp, 4 * 4
	pop_step
	; store new particle state
	store_particle
	; restore working registers
	pop		rsi
	pop		rcx
	pop		rbx

	advance_particle_ptr

	dec		ecx
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
	%arg		emitter:dword
	%arg		startPtr:dword
	%arg		endPtr:dword
	%arg		step:dword
	%arg		cameraCS:dword
	%arg		buffer:dword
	%arg		maxVertices:dword
	%local		verts:dword

	enter   %$localsize, 0

	push	rbx
	push	rsi
	push	rdi

	mov		ebx, [maxVertices]
	mov		[verts], ebx

	; initialize pointers
	mov		esi, [startPtr]
	mov		edi, [endPtr]
	mov		ebx, [emitter]

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
	push	MASK_ALPHA
	push	MASK_RGB
	push	dword INV_RAND_MAX
	push	rand
	mov		ebx, [emitter]

	; call the processing code
	call	[ebx + ptcEmitter.InternalPtr3]

	; restore previous state
	add		esp, 4 * 4
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
	mov		edx, [maxVertices]
	test	edx, edx
	jz		.end
	mov		edi, [buffer]
	; just copy the colour into an xmm
	movups	xmm7, [esi + ptcParticle.Colour]
	; find the vertices of the particle
	movups	xmm0, [esi + ptcParticle.Location]
	; load in the particle size and broadcast it to all components
	movss	xmm3, [esi + ptcParticle.Size]
	shufps	xmm3, xmm3, 0x00
	; load in 0.5 and broadcast it to all components
	push	dword 0x3F000000
	movss	xmm4, [esp]
	add		esp, 4
	shufps	xmm4, xmm4, 0x00
	; load in the camera coordinate system
	push	rsi
	mov		esi, [cameraCS]
	movups	xmm1, [esi + 3 * 4]
	movups	xmm2, [esi + 6 * 4]
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
	movups	[edi + ptcVertex_size * 0 + ptcVertex.Colour], xmm7
	movlps	[edi + ptcVertex_size * 0 + ptcVertex.Location], xmm3
	shufps	xmm3, xmm3, 0xAA
	movss	[edi + ptcVertex_size * 0 + ptcVertex.Location + 8], xmm3
	mov		[edi + ptcVertex_size * 0 + ptcVertex.TexCoords], word 1
	mov		[edi + ptcVertex_size * 0 + ptcVertex.TexCoords + 2], word 0

	movups	[edi + ptcVertex_size * 1 + ptcVertex.Colour], xmm7
	movlps	[edi + ptcVertex_size * 1 + ptcVertex.Location], xmm4
	shufps	xmm4, xmm4, 0xAA
	movss	[edi + ptcVertex_size * 1 + ptcVertex.Location + 8], xmm4
	mov		[edi + ptcVertex_size * 1 + ptcVertex.TexCoords], word 0
	mov		[edi + ptcVertex_size * 1 + ptcVertex.TexCoords + 2], word 0

	movups	[edi + ptcVertex_size * 2 + ptcVertex.Colour], xmm7
	movlps	[edi + ptcVertex_size * 2 + ptcVertex.Location], xmm5
	shufps	xmm5, xmm5, 0xAA
	movss	[edi + ptcVertex_size * 2 + ptcVertex.Location + 8], xmm5
	mov		[edi + ptcVertex_size * 2 + ptcVertex.TexCoords], word 0
	mov		[edi + ptcVertex_size * 2 + ptcVertex.TexCoords + 2], word 1

	movups	[edi + ptcVertex_size * 3 + ptcVertex.Colour], xmm7
	movlps	[edi + ptcVertex_size * 3 + ptcVertex.Location], xmm6
	shufps	xmm6, xmm6, 0xAA
	movss	[edi + ptcVertex_size * 3 + ptcVertex.Location + 8], xmm6
	mov		[edi + ptcVertex_size * 3 + ptcVertex.TexCoords], word 1
	mov		[edi + ptcVertex_size * 3 + ptcVertex.TexCoords + 2], word 1

	; advance pointers
	sub		edx, 4
	mov		[maxVertices], edx
	add		edi, ptcVertex_size * 4
	mov		[buffer], edi
	pop		rdi
.cont:
	add		esi, ptcParticle_size
	cmp		esi, edi
	jl		.loop

.end:
	mov		ebx, [maxVertices]
	mov		eax, [verts]
	sub		eax, ebx

	pop		rdi
	pop		rbx

	leave
	ret
	%pop
