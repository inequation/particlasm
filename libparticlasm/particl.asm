; particlasm main module
; Copyright (C) 2011-2012, Leszek Godlewski <lg@inequation.org>

; NASMX base include
%include 'nasmx.inc'

; some NASMX supplements
%ixdefine ptr_t				ptrdiff_t			; platform-specific pointer type
%ixdefine ptr_t_reserver	ptrdiff_t_reserve
%ixdefine float_s_reserve	__nxfloat_reserve	; 32-bit float reservation
%ixdefine float_s			dword
%ixdefine float_size		__nxfloat_size
%ifidn __BITS__,64
	%ixdefine ptr_t_size	qword_size
	%ixdefine dummy_addr	0x175ABADADD		; 40-bit address just to make sure we overflow a dword
%else
	%ixdefine ptr_t_size	dword_size
	%ixdefine dummy_addr	0x15BADADD		; 32-bit address just to make sure we utilize a whole dword
%endif

; libparticlasm declarations generated from the C header
%include "libparticlasm.inc"

; by default, all addresses are absolute
default abs

; libc imports
extern rand
extern memcpy

; declare the GOT symbol
extern _GLOBAL_OFFSET_TABLE_
; macro used to store the _GLOBAL_OFFSET_TABLE_ address in ebx
%macro get_GOT 0
	call	%%getgot
%%getgot:
	pop		__bx
	add		__bx, _GLOBAL_OFFSET_TABLE_+$$-%%getgot wrt ..gotpc
%endmacro

%ifidni __OUTPUT_FORMAT__,win64
	%ixdefine __WINDOWS__	1
%elifidni __OUTPUT_FORMAT__,win32
	%ixdefine __WINDOWS__	1
%else
	%ixdefine __WINDOWS__	0
%endif
%if !(__WINDOWS__)
	; entry points to the library
	global ptcInternalMeasureModule:function
	global ptcInternalCompileModule:function
	global ptcInternalSpawnParticles:function
	global ptcInternalProcessParticles:function
%endif

; equal to 1.f / RAND_MAX (as per the stdlib.h declaration) in hex representation
%define INV_RAND_MAX	0x30000000	;4.65661287524579692411e-10

; utility macro that places a random number in the [0..1] range in st0
; %1 - additional stack offset, 0 by default
%macro frand 0-1 0
	; need to make the call a far one since the code will live in a totally
	; different memory block
	call	[__sp + (4 + %1)]
	; temp variable - the fild instruction needs to read from memory
	push	__ax
	fild	float_s [__sp]
	add		__sp, 4
	; load in the inverse RAND_MAX
	fld		float_s [__sp + (8 + %1)]
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
	mov		__ax, [var(.spawnbuf_len)]
	add		__ax, __bx
	mov		[var(.spawnbuf_len)], __ax
	mov		__ax, [var(.procbuf_len)]
	add		__ax, __cx
	mov		[var(.procbuf_len)], __ax
	mov		__ax, [var(.databuf_len)]
	add		__ax, __dx
	mov		[var(.databuf_len)], __ax
	jmp		.end
%%skip:
%endmacro

; macro to compile module code into the given buffer pointer
%macro CompileMod 1
	cmp		edx, ptcMID_ %+ %1
	jne		%%skip
	invoke	mod_ %+ %1 %+ Compile, [var(.dataBuf)], [var(.processCodeBuf)], [var(.spawnCodeBuf)]
	jne		.end
%%skip:
%endmacro

; loads particle data into registers; assumes pointer in rsi
%macro load_particle 0
	mov		edx, [__si + ptcParticle.Active]
	fld		float_s [__si + ptcParticle.Size]
	fld		float_s [__si + ptcParticle.Rotation]
	fld		float_s [__si + ptcParticle.Time]
	fld		float_s [__si + ptcParticle.TimeScale]
	movups	xmm1, [__si + ptcParticle.Colour]
	movups	xmm2, [__si + ptcParticle.Location]
	movups	xmm3, [__si + ptcParticle.Velocity]
	movups	xmm4, [__si + ptcParticle.Accel]
%endmacro

; stores particle data into memory; assumes pointer in esi
%macro store_particle 0
	; our vectors are 12 bytes wide, so these stores require some SSE magic:
	; first store the low order half of the register, then broadcast the 3rd
	; float onto the entire register and store it as a scalar at an offset
	movlps	[__si + ptcParticle.Accel], xmm4
	shufps	xmm4, xmm4, 0xAA
	movss	[__si + ptcParticle.Accel + 8], xmm4
	movlps	[__si + ptcParticle.Velocity], xmm3
	shufps	xmm3, xmm3, 0xAA
	movss	[__si + ptcParticle.Velocity + 8], xmm3
	movlps	[__si + ptcParticle.Location], xmm2
	shufps	xmm2, xmm2, 0xAA
	movss	[__si + ptcParticle.Location + 8], xmm2
	; colour is 16 bytes wide, no magic required
	movups	[__si + ptcParticle.Colour], xmm1
	fstp	float_s [__si + ptcParticle.TimeScale]
	fstp	float_s [__si + ptcParticle.Time]
	fstp	float_s [__si + ptcParticle.Rotation]
	fstp	float_s [__si + ptcParticle.Size]
	mov		[__si + ptcParticle.Active], edx
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
	add		__si, ptcParticle_size
	mov		__dx, [__bx + ptcEmitter.MaxParticles]
	mov		__ax, ptcParticle_size
	mul		__dx
	mov		__dx, [__bx + ptcEmitter.ParticleBuf]
	add		__ax, __dx
	cmp		__si, __ax
	jl		%%skip
	mov		__si, __dx
%%skip:
%endmacro

proc ptcInternalMeasureModule, ptr_t module, ptr_t spawnCodeBufLenPtr, ptr_t processCodeBufLenPtr, ptr_t dataBufLenPtr
uses __ax, __bx, __cx, __dx, __si, __di	; every register is guaranteed to be used sooner or later
locals
	local spawnbuf_len,	size_t	; particle spawn code buffer size
	local procbuf_len,	size_t	; particle processing code buffer size
	local databuf_len,	size_t	; particle data buffer size
endlocals

; spill registers to stack on x64
%ifidn __OUTPUT_FORMAT__,elf64
	; rdi = module, rsi = spawn..., rdx = process..., rcx = data...
	mov		[argv(.module)], rdi
	mov		[argv(.spawnCodeBufLenPtr)], rsi
	mov		[argv(.processCodeBufLenPtr)], rdx
	mov		[argv(.dataBufLenPtr)], rcx
%elifidn __OUTPUT_FORMAT__,win64
	; rcx = module, rdx = spawn..., r8 = process..., r9 = data...
	mov		[argv(.module)], rcx
	mov		[argv(.spawnCodeBufLenPtr)], rdx
	mov		[argv(.processCodeBufLenPtr)], r8
	mov		[argv(.dataBufLenPtr)], r9
%endif

	; initialize counters
	mov		__si, [argv(.spawnCodeBufLenPtr)]
	mov		__dx, [__si]
	mov		[var(.spawnbuf_len)], __dx
	mov		__si, [argv(.processCodeBufLenPtr)]
	mov		__dx, [__si]
	mov		[var(.procbuf_len)], __dx
	mov		__si, [argv(.dataBufLenPtr)]
	mov		__dx, [__si]
	mov		[var(.databuf_len)], __dx

	mov		__si, [argv(.module)]

	; if module == NULL, add in the simulation module
	test	__si, __si
	jnz		.normal

	mov		__ax, [var(.spawnbuf_len)]
	inc		__ax	; add in 1 byte for a ret instruction
	mov		[var(.spawnbuf_len)], __ax
	mov		__ax, [var(.procbuf_len)]
	add		__ax, mod_SimulatePre_size + mod_SimulatePost_size
	mov		[var(.procbuf_len)], __ax
	jmp		.end

.normal:
	mov		__dx, [__si + ptcModuleHeader.ModuleID]

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
	mov		__dx, [var(.spawnbuf_len)]
	mov		__di, [argv(.spawnCodeBufLenPtr)]
	mov		[__di], __dx
	mov		__dx, [var(.procbuf_len)]
	mov		__di, [argv(.processCodeBufLenPtr)]
	mov		[__di], __dx
	mov		__dx, [var(.databuf_len)]
	mov		__di, [argv(.dataBufLenPtr)]
	mov		[__di], __dx

endproc

proc ptcInternalCompileModule, ptr_t module, ptr_t spawnCodeBufPtr, ptr_t processCodeBufPtr, ptr_t dataBufPtr
uses __bx, __si, __di
locals
	local	dataBuf,		ptr_t
	local	processCodeBuf,	ptr_t
	local	spawnCodeBuf,	ptr_t
endlocals

; spill registers to stack on x64
%ifidn __OUTPUT_FORMAT__,elf64
	; rdi = module, rsi = spawn..., rdx = process..., rcx = data...
	mov		[argv(.module)], rdi
	mov		[argv(.spawnCodeBufPtr)], rsi
	mov		[argv(.processCodeBufPtr)], rdx
	mov		[argv(.dataBufPtr)], rcx
%elifidn __OUTPUT_FORMAT__,win64
	; rcx = module, rdx = spawn..., r8 = process..., r9 = data...
	mov		[argv(.module)], rcx
	mov		[argv(.spawnCodeBufPtr)], rdx
	mov		[argv(.processCodeBufPtr)], r8
	mov		[argv(.dataBufPtr)], r9
%endif

	; place the pointers on the stack in the convention that modules expect them
	mov		__ax, [argv(.dataBufPtr)]
	mov		__ax, [__ax]
	mov		[var(.dataBuf)], __ax
	mov		__ax, [argv(.processCodeBufPtr)]
	mov		__ax, [__ax]
	mov		[var(.processCodeBuf)], __ax
	mov		__ax, [argv(.spawnCodeBufPtr)]
	mov		__ax, [__ax]
	mov		[var(.spawnCodeBuf)], __ax
	mov		__si, [argv(.module)]

	; check for simulation pre- and post-processing modules
	cmp		__si, ptr_t -1
	jne		.try_post
	; add in the simulation preprocessing module
	invoke	mod_SimulatePreCompile, [var(.dataBuf)], [var(.processCodeBuf)], [var(.spawnCodeBuf)]
	jmp		.end
.try_post:
	cmp		__si, ptr_t -2
	jne		.normal
	; add in the simulation postprocessing module
	invoke	mod_SimulatePostCompile, [var(.dataBuf)], [var(.processCodeBuf)], [var(.spawnCodeBuf)]
	jmp		.end
.normal:
	mov		__dx, [__si + ptcModuleHeader.ModuleID]

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
	pop		__dx
	mov		__ax, [argv(.spawnCodeBufPtr)]
	mov		[__ax], __dx
	pop		__dx
	mov		__ax, [argv(.processCodeBufPtr)]
	mov		[__ax], __dx
	pop		__dx
	mov		__ax, [argv(.dataBufPtr)]
	mov		[__ax], __dx

endproc

proc ptcInternalSpawnParticles, ptr_t emitter, float_t step, size_t count
uses __bx, __si, __di
locals none

; spill registers to stack on x64
%ifidn __OUTPUT_FORMAT__,elf64
	; rdi = emitter, xmm0 = step, rsi = count
	mov		[argv(.emitter)], rdi
	movss	[argv(.step)], xmm0
	mov		[argv(.count)], rsi
%elifidn __OUTPUT_FORMAT__,win64
	; rcx = emitter, xmm1 = step, r8 = count
	mov		[argv(.emitter)], rcx
	movss	[argv(.step)], xmm1
	mov		[argv(.count)], r8
%endif

	; initialize the FPU to make sure the stack is clear and there are no
	; exceptions
	finit
	fwait

	; start at a random index to reduce chances of a collision
	call	rand wrt ..plt
	mov		__bx, [argv(.emitter)]
	xor		__dx, __dx
	mov		__cx, [__bx + ptcEmitter.MaxParticles]
	div		__cx
	mov		__ax, ptcParticle_size
	mul		__dx
	mov		__si, __ax

	mov		__cx, [argv(.count)]
	add		__si, [__bx + ptcEmitter.ParticleBuf]

	; kick off the loop - find a free spot in the buffer
.find_spot:
	mov		edx, [__si + ptcParticle.Active]
	test	edx, edx	; intentionally always 32-bit
	jz		.cont
	advance_particle_ptr
	jmp		.find_spot
.cont:
	; increase particle counter
	mov		__dx, [__bx + ptcEmitter.NumParticles]
	inc		__dx;
	mov		[__bx + ptcEmitter.NumParticles], __dx
	; clear particle data
	mov		[__si + ptcParticle.Active], dword 1
	; calculate time scale: 1 / (LifeTimeFixed + frand() * LifeTimeRandom)
	fld1
	fld		float_s [__bx + ptcEmitterConfig.LifeTimeFixed]
	fld		float_s [__bx + ptcEmitterConfig.LifeTimeRandom]
	; save off rcx and rdx
	push	__cx
	push	__dx
	; need the function address and inverse RAND_MAX on the stack
	push	dword INV_RAND_MAX
%ifidn __BITS__,64
	push	ptr_t [rel rand wrt ..plt]
%else
	push	rand
%endif
	push	0	; dummy value
	frand
	add		__sp, 4 * 3
	; restore ecx and edx
	pop		__dx
	pop		__cx
	; st0=frand(), st1=LTR, st2=LTF, st3=1.0
	fmulp	st1, st0
	; st0=frand()*LTR, st1=LTF, st2=1.0
	faddp	st1, st0
	; st0=LTF+frand()*LTR, st1=1.0
	fdivp	st1, st0
	; st0=1.0/t
	fstp	float_s [__si + ptcParticle.TimeScale]
	; sync
	fwait
	mov		[__si + ptcParticle.Time], float_s 0
	fld1
	fstp	float_s [__si + ptcParticle.Colour]
	fwait
	; reuse the 1.0 float we've just stored
	mov		eax, float_s [__si + ptcParticle.Colour]
	mov		float_s [esi + (ptcParticle.Colour + 4)], eax
	mov		float_s [esi + (ptcParticle.Colour + 8)], eax
	mov		float_s [esi + (ptcParticle.Colour + 12)], eax
	mov		[__si + ptcParticle.Location], float_s 0
	mov		[__si + (ptcParticle.Location + 4)], float_s 0
	mov		[__si + (ptcParticle.Location + 8)], float_s 0
	mov		[__si + ptcParticle.Rotation], float_s 0
	mov		float_s [__si + ptcParticle.Size], eax
	mov		[__si + ptcParticle.Velocity], float_s 0
	mov		[__si + (ptcParticle.Velocity + 4)], float_s 0
	mov		[__si + (ptcParticle.Velocity + 8)], float_s 0
	mov		[__si + ptcParticle.Accel], float_s 0
	mov		[__si + (ptcParticle.Accel + 4)], float_s 0
	mov		[__si + (ptcParticle.Accel + 8)], float_s 0
	; save off working registers
	push	__bx
	push	__cx
	push	__si
	; load particle data into registers
	load_particle
	push_step [argv(.step)]
	; load the pointers that we need
%ifidn __BITS__,64
	push	ptr_t [rel MASK_ALPHA]
	push	ptr_t [rel MASK_RGB]
%else
	push	MASK_ALPHA
	push	MASK_RGB
%endif
	push	dword INV_RAND_MAX
%ifidn __BITS__,64
	push	ptr_t [rel rand wrt ..plt]
%else
	push	rand
%endif
	mov		__bx, [argv(.emitter)]

	; call the spawn code
	call	[__bx + ptcEmitter.InternalPtr2]

	; mark as active
	mov		edx, dword 1

	; restore previous state
	add		__sp, 4 * 4
	pop_step
	; store new particle state
	store_particle
	; restore working registers
	pop		__si
	pop		__cx
	pop		__bx

	advance_particle_ptr

	dec		__cx
	jecxz	.end
	jmp		.find_spot
.end:

endproc

proc ptcInternalProcessParticles, ptr_t emitter, ptr_t startPtr, ptr_t endPtr, float_t step, ptr_t cameraCS, ptr_t buffer, ptr_t maxVertices
uses __bx, __si, __di
locals
	local	verts, ptr_t
endlocals

; spill registers to stack on x64
%ifidn __OUTPUT_FORMAT__,elf64
	; rdi = emitter, rsi = startPtr, rdx = endPtr, xmm0 = step, rcx = cameraCS, r8 = buffer, r9 = maxVertices
	mov		[argv(.emitter)], rdi
	mov		[argv(.startPtr)], rsi
	mov		[argv(.endPtr)], rdx
	movss	[argv(.step)], xmm0
	mov		[argv(.cameraCS)], rcx
	mov		[argv(.buffer)], r8
	mov		[argv(.maxVertices)], r9
%elifidn __OUTPUT_FORMAT__,win64
	; rcx = emitter, rdx = startPtr, r8 = endPtr, xmm3 = step, the rest is on the stack already
	mov		[argv(.emitter)], rcx
	mov		[argv(.startPtr)], rdx
	mov		[argv(.endPtr)], r8
	movss	[argv(.step)], xmm3
%endif

	mov		__bx, [argv(.maxVertices)]
	mov		[var(.verts)], __bx

	; initialize pointers
	mov		__si, [argv(.startPtr)]
	mov		__di, [argv(.endPtr)]
	mov		__bx, [argv(.emitter)]

	; kick off the loop
.loop:
	; save off working registers
	push	__bx
	push	__cx
	push	__si
	push	__di
	; load particle data into registers
	load_particle
	push_step [argv(.step)]
	; load the pointers that we need
%ifidn __BITS__,64
	push	ptr_t [rel MASK_ALPHA]
	push	ptr_t [rel MASK_RGB]
%else
	push	MASK_ALPHA
	push	MASK_RGB
%endif
	push	dword INV_RAND_MAX
%ifidn __BITS__,64
	push	ptr_t [rel rand wrt ..plt]
%else
	push	rand
%endif
	mov		__bx, [argv(.emitter)]

	; call the processing code
	call	[__bx + ptcEmitter.InternalPtr3]

	; restore previous state
	add		__sp, 4 * 4
	pop_step
	; store new particle state
	store_particle
	; restore working registers
	pop		__di
	pop		__si
	pop		__cx
	pop		__bx

	; skip vertex emitting if there's nothing to emit it for
	test	edx, edx
	jz		.cont

	push	__di

	; emit vertices
	mov		__dx, [argv(.maxVertices)]
	test	__dx, __dx
	jz		.end
	mov		__di, [argv(.buffer)]
	; just copy the colour into an xmm
	movups	xmm7, [__si + ptcParticle.Colour]
	; find the vertices of the particle
	movups	xmm0, [__si + ptcParticle.Location]
	; load in the particle size and broadcast it to all components
	movss	xmm3, [__si + ptcParticle.Size]
	shufps	xmm3, xmm3, 0x00
	; load in 0.5 and broadcast it to all components
	push	float_s 0x3F000000
	movss	xmm4, [__sp]
	add		__sp, 4
	shufps	xmm4, xmm4, 0x00
	; load in the camera coordinate system
	push	__si
	mov		__si, [argv(.cameraCS)]
	movups	xmm1, [__si + 3 * sizeof(float_t)]
	movups	xmm2, [__si + 6 * sizeof(float_t)]
	pop		__si
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
	movups	[__di + ptcVertex_size * 0 + ptcVertex.Colour], xmm7
	movlps	[__di + ptcVertex_size * 0 + ptcVertex.Location], xmm3
	shufps	xmm3, xmm3, 0xAA
	movss	[__di + ptcVertex_size * 0 + ptcVertex.Location + 8], xmm3
	mov		[__di + ptcVertex_size * 0 + ptcVertex.TexCoords], word 1
	mov		[__di + ptcVertex_size * 0 + ptcVertex.TexCoords + 2], word 0

	movups	[__di + ptcVertex_size * 1 + ptcVertex.Colour], xmm7
	movlps	[__di + ptcVertex_size * 1 + ptcVertex.Location], xmm4
	shufps	xmm4, xmm4, 0xAA
	movss	[__di + ptcVertex_size * 1 + ptcVertex.Location + 8], xmm4
	mov		[__di + ptcVertex_size * 1 + ptcVertex.TexCoords], word 0
	mov		[__di + ptcVertex_size * 1 + ptcVertex.TexCoords + 2], word 0

	movups	[__di + ptcVertex_size * 2 + ptcVertex.Colour], xmm7
	movlps	[__di + ptcVertex_size * 2 + ptcVertex.Location], xmm5
	shufps	xmm5, xmm5, 0xAA
	movss	[__di + ptcVertex_size * 2 + ptcVertex.Location + 8], xmm5
	mov		[__di + ptcVertex_size * 2 + ptcVertex.TexCoords], word 0
	mov		[__di + ptcVertex_size * 2 + ptcVertex.TexCoords + 2], word 1

	movups	[__di + ptcVertex_size * 3 + ptcVertex.Colour], xmm7
	movlps	[__di + ptcVertex_size * 3 + ptcVertex.Location], xmm6
	shufps	xmm6, xmm6, 0xAA
	movss	[__di + ptcVertex_size * 3 + ptcVertex.Location + 8], xmm6
	mov		[__di + ptcVertex_size * 3 + ptcVertex.TexCoords], word 1
	mov		[__di + ptcVertex_size * 3 + ptcVertex.TexCoords + 2], word 1

	; advance pointers
	sub		__dx, 4
	mov		[argv(.maxVertices)], __dx
	add		__di, ptcVertex_size * 4
	mov		[argv(.buffer)], __di
	pop		__di
.cont:
	add		__si, ptcParticle_size
	cmp		__si, __di
	jl		.loop

.end:
	mov		__bx, [argv(.maxVertices)]
	mov		__ax, [var(.verts)]
	sub		__ax, __bx

endproc
