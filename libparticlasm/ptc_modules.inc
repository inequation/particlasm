; particlasm modules code
; Copyright (C) 2011-2012, Leszek Godlewski <lg@inequation.org>

%ifndef PTC_MODULES
%define PTC_MODULES
; declarations
%include "libparticlasm.inc"

; libc memcpy
extern memcpy

; Spawning and processing calling convention:
; edx - particle active or not
; [esp + 4] - pointer to rand() (as a workaround)
; [esp + 8] - INV_RAND_MAX
; [esp + 12] - pointer to MASK_RGB
; [esp + 16] - pointer to MASK_ALPHA
;
; NOTE: Spawning expects the FPU stack to be cleared (zeroed) so that values can
;       be placed in the registers by simple addition
; st0 - simulation step
; st1 - ptcParticle.TimeScale
; st2 - ptcParticle.Time
; st3 - ptcParticle.Rotation
; st4 - ptcParticle.Size
;
; xmm0 - simulation step (as a vector, i.e. [st0, st0, st0, st0])
; xmm1 - ptcParticle.Colour
; xmm2 - ptcParticle.Location
; xmm3 - ptcParticle.Velocity
; xmm4 - ptcParticle.Accel

; Measurement calling convention:
; esi - pointer to module
; ebx - spawn code size in bytes returned
; ecx - processing code size in bytes returned
; edx - data size in bytes returned

; Compilation calling convention:
; (esp + 4) - working pointer to spawn code buffer
; (esp + 8) - working pointer to processing code buffer
; (esp + 12) - working pointer to data buffer

; ========================================
; Particle simulation module
; ========================================

; simulation module, preprocessing
mod_SimulatePre:
	; if the particle is not active, just skip it
	test	edx, edx
	jz		.return
	; T += t * Ts
	fld		st0
	fmul	st0, st2
	faddp	st3, st0
	; kill the particle if we're past its lifetime
	fld1
	fcomip	st3
	fwait
	; if it's still < 1, skip to the end
	jae		.end
	; decrease particle counter
	mov		edx, [ebx + ptcEmitter.NumParticles]
	dec		edx;
	mov		[ebx + ptcEmitter.NumParticles], edx
	xor		edx, edx
.return:
	ret
.end:
mod_SimulatePre_size	equ	(mod_SimulatePre.end - mod_SimulatePre)

mod_SimulatePreCompile:
	mov		eax, [esp + 8]
	push	mod_SimulatePre_size
	push	mod_SimulatePre
	push	eax
	call	memcpy
	add		esp, 4 * 3
	add		eax, mod_SimulatePre_size
	mov		[esp + 8], eax
ret

; simulation module, postprocessing
mod_SimulatePost:
	; v += a * t
	movaps	xmm7, xmm4
	mulps	xmm7, xmm0
	addps	xmm3, xmm7
	; x += v * t
	movaps	xmm7, xmm3
	mulps	xmm7, xmm0
	addps	xmm2, xmm7
	ret
.end:
mod_SimulatePost_size	equ	(mod_SimulatePost.end - mod_SimulatePost)

mod_SimulatePostCompile:
	mov		edi, [esp + 8]
	; copy in the postprocessing code
	push	dword mod_SimulatePost_size
	push	dword mod_SimulatePost
	push	edi
	call	memcpy
	add		esp, 4 * 3
	add		edi, mod_SimulatePost_size
	mov		[esp + 8], edi

	; also drop in a ret instruction at the end of spawn code
	mov		edi, [esp + 4]
	mov		edx, [mod_SimulatePostCompile.ret_instruction]
	mov		[edi], byte dl
	inc		edi
	mov		[esp + 4], edi
.ret_instruction:
	ret

; ========================================
; InitialLocation
; ========================================

mod_InitialLocationSpawn:
	; distribution code goes here
	; move distribution result to target register
	movups xmm2, xmm5
.end:
mod_InitialLocationSpawn_size	equ	(mod_InitialLocationSpawn.end - mod_InitialLocationSpawn)

; nothing to process
mod_InitialLocationProcess:
.end:

mod_InitialLocationMeasure:
	mov		eax, [esi + ptcMod_InitialLocation.Distr]
	distr_size eax, ebx, edx, v
	add		ebx, (mod_InitialLocationSpawn.end - mod_InitialLocationSpawn)
	xor		ecx, ecx
	ret

mod_InitialLocationCompile:
	; prepare pointers
	mov		ecx, [esp + 4]
	mov		edi, [esp + 12]

	; compile distribution
	add		esi, ptcMod_InitialLocation.Distr
	mov		eax, [esi]
	distr_compile eax, v
	sub		esi, ptcMod_InitialLocation.Distr

	; save off ecx
	push	ecx

	; copy our code over
	mov		edx, mod_InitialLocationSpawn_size
	push	edx
	push	dword mod_InitialLocationSpawn
	push	ecx
	call	memcpy
	add		esp, 4 * 3

	; restore ecx
	pop		ecx

	; advance and save pointers
	add		ecx, mod_InitialLocationSpawn_size
	mov		[esp + 12], edi
	mov		[esp + 4], ecx
	ret

; ========================================
; InitialRotation
; ========================================

mod_InitialRotationSpawn:
	; distribution code goes here
	; move distribution result to target register
	faddp st4, st0
.end:
mod_InitialRotationSpawn_size	equ (mod_InitialRotationSpawn.end - mod_InitialRotationSpawn)

; nothing to process
mod_InitialRotationProcess:
.end:

mod_InitialRotationMeasure:
	mov		eax, [esi + ptcMod_InitialRotation.Distr]
	distr_size eax, ebx, edx, s
	add		ebx, (mod_InitialRotationSpawn.end - mod_InitialRotationSpawn)
	xor		ecx, ecx
	ret

mod_InitialRotationCompile:
	; prepare pointers
	mov		ecx, [esp + 4]
	mov		edi, [esp + 12]

	; compile distribution
	add		esi, ptcMod_InitialRotation.Distr
	mov		eax, [esi]
	distr_compile eax, s
	sub		esi, ptcMod_InitialRotation.Distr

	; save off ecx
	push	ecx

	; copy our code over
	mov		edx, mod_InitialRotationSpawn_size
	push	edx
	push	dword mod_InitialRotationSpawn
	push	ecx
	call	memcpy
	add		esp, 4 * 3

	; restore ecx
	pop		ecx

	; advance and save pointers
	add		ecx, mod_InitialRotationSpawn_size
	mov		[esp + 12], edi
	mov		[esp + 4], ecx
	ret

; ========================================
; InitialSize
; ========================================

mod_InitialSizeSpawn:
	; distribution code goes here
	; move distribution result to target register
	fldz
	fxch	st5, st0
	fstp	st0
	faddp	st5, st0
.end:
mod_InitialSizeSpawn_size	equ	(mod_InitialSizeSpawn.end - mod_InitialSizeSpawn)

; nothing to process
mod_InitialSizeProcess:
.end:

mod_InitialSizeMeasure:
	mov		eax, [esi + ptcMod_InitialSize.Distr]
	distr_size eax, ebx, edx, s
	add		ebx, (mod_InitialSizeSpawn.end - mod_InitialSizeSpawn)
	xor		ecx, ecx
	ret

mod_InitialSizeCompile:
	; prepare pointers
	mov		ecx, [esp + 4]
	mov		edi, [esp + 12]

	; compile distribution
	add		esi, ptcMod_InitialSize.Distr
	mov		eax, [esi]
	distr_compile eax, s
	sub		esi, ptcMod_InitialSize.Distr

	; save off ecx
	push	ecx

	; copy our code over
	mov		edx, mod_InitialSizeSpawn_size
	push	edx
	push	dword mod_InitialSizeSpawn
	push	ecx
	call	memcpy
	add		esp, 4 * 3

	; restore ecx
	pop		ecx

	; advance and save pointers
	add		ecx, mod_InitialSizeSpawn_size
	mov		[esp + 12], edi
	mov		[esp + 4], ecx
	ret

; ========================================
; InitialVelocity
; ========================================

mod_InitialVelocitySpawn:
	; distribution code goes here
	; move distribution result to target register
	movaps	xmm3, xmm5
.end:
mod_InitialVelocitySpawn_size	equ	(mod_InitialVelocitySpawn.end - mod_InitialVelocitySpawn)

; nothing to process
mod_InitialVelocityProcess:
.end:

mod_InitialVelocityMeasure:
	mov		eax, [esi + ptcMod_InitialVelocity.Distr]
	distr_size eax, ebx, edx, v
	add		ebx, mod_InitialVelocitySpawn_size
	xor		ecx, ecx
	ret

mod_InitialVelocityCompile:
	; prepare pointers
	mov		ecx, [esp + 4]
	mov		edi, [esp + 12]

	; compile distribution
	add		esi, ptcMod_InitialVelocity.Distr
	mov		eax, [esi]
	distr_compile eax, v
	sub		esi, ptcMod_InitialVelocity.Distr

	; save off ecx
	push	ecx

	; copy our code over
	mov		edx, mod_InitialVelocitySpawn_size
	push	edx
	push	dword mod_InitialVelocitySpawn
	push	ecx
	call	memcpy
	add		esp, 4 * 3

	; restore ecx
	pop		ecx

	; advance and save pointers
	add		ecx, mod_InitialVelocitySpawn_size
	mov		[esp + 12], edi
	mov		[esp + 4], ecx
	ret

; ========================================
; InitialColour
; ========================================

mod_InitialColourSpawn:
	; load colour flags
	mov eax, [0xFFFFFFFF]	; filled upon compilation
.predistr:
	; distribution code goes here
.postdistr:
	; move distribution result to target register
	movaps xmm1, xmm5
.end:

; nothing to process
mod_InitialColourProcess:
.end:

mod_InitialColourMeasure:
	mov		eax, [esi + ptcMod_InitialColour.Distr]
	distr_size eax, ebx, edx, c
	add		ebx, (mod_InitialColourSpawn.end - mod_InitialColourSpawn)
	; also add the flags to the data length
	add		edx, 4
	xor		ecx, ecx
	ret

mod_InitialColourCompile:
	; prepare pointers
	mov		ecx, [esp + 4]
	mov		edi, [esp + 12]

	; save off ecx
	push	ecx

	; copy our pre-distribution code over
	push	dword (mod_InitialColourSpawn.predistr - mod_InitialColourSpawn)
	push	mod_InitialColourSpawn
	push	ecx
	call	memcpy
	add		esp, 4 * 3

	; restore ecx
	pop		ecx
	; fill in the read address
	mov		[ecx + (mod_InitialColourSpawn.predistr - mod_InitialColourSpawn - 4)], edi
	; put the fake flags in the data buffer
	mov		[edi], dword (ptcCF_SetAlpha | ptcCF_SetRGB)
	; advance the pointers
	add		ecx, (mod_InitialColourSpawn.predistr - mod_InitialColourSpawn)
	add		edi, 4

	; compile distribution
	add		esi, ptcMod_InitialColour.Distr
	mov		eax, [esi]
	distr_compile eax, c
	sub		esi, ptcMod_InitialColour.Distr

	; save off ecx
	push	ecx

	; copy our post-distribution code over
	mov		edx, (mod_InitialColourSpawn.end - mod_InitialColourSpawn.postdistr)
	push	edx
	push	dword mod_InitialColourSpawn.postdistr
	push	ecx
	call	memcpy
	add		esp, 4 * 3

	; restore ecx
	pop		ecx

	; advance and save pointers
	add		ecx, (mod_InitialColourSpawn.end - mod_InitialColourSpawn.postdistr)
	mov		[esp + 12], edi
	mov		[esp + 4], ecx
	ret

; ========================================
; Velocity
; ========================================

; nothing to do on spawn
mod_VelocitySpawn:
.end:

mod_VelocityProcess:
	; distribution code goes here
	; move distribution result to target register
	movaps xmm3, xmm5
.end:
mod_VelocityProcess_size	equ	(mod_VelocityProcess.end - mod_VelocityProcess)

mod_VelocityMeasure:
	mov		eax, [esi + ptcMod_Velocity.Distr]
	distr_size eax, ecx, edx, v
	add		ecx, (mod_VelocityProcess.end - mod_VelocityProcess)
	xor		ebx, ebx
	ret

mod_VelocityCompile:
	; prepare pointers
	mov		ecx, [esp + 8]
	mov		edi, [esp + 12]

	; compile distribution
	add		esi, ptcMod_Velocity.Distr
	mov		eax, [esi]
	distr_compile eax, v
	sub		esi, ptcMod_Velocity.Distr

	; save off ecx
	push	ecx

	; copy our code over
	mov		edx, mod_VelocityProcess_size
	push	edx
	push	dword mod_VelocityProcess
	push	ecx
	call	memcpy
	add		esp, 4 * 3

	; restore ecx
	pop		ecx

	; advance and save pointers
	add		ecx, mod_VelocityProcess_size
	mov		[esp + 12], edi
	mov		[esp + 8], ecx
	ret

; ========================================
; Acceleration
; ========================================

; nothing to do on spawn
mod_AccelerationSpawn:
.end:

mod_AccelerationProcess:
	; distribution code goes here
	; move distribution result to target register
	movaps xmm4, xmm5
.end:
mod_AccelerationProcess_size	equ	(mod_AccelerationProcess.end - mod_AccelerationProcess)

mod_AccelerationMeasure:
	mov eax, [esi + ptcMod_Acceleration.Distr]
	distr_size eax, ecx, edx, v
	add ecx, (mod_AccelerationProcess.end - mod_AccelerationProcess)
	xor	ebx, ebx
	ret

mod_AccelerationCompile:
	; prepare pointers
	mov		ecx, [esp + 8]
	mov		edi, [esp + 12]

	; compile distribution
	add		esi, ptcMod_Acceleration.Distr
	mov		eax, [esi]
	distr_compile eax, v
	sub		esi, ptcMod_Acceleration.Distr

	; save off ecx
	push	ecx

	; copy our code over
	mov		edx, mod_AccelerationProcess_size
	push	edx
	push	dword mod_AccelerationProcess
	push	ecx
	call	memcpy
	add		esp, 4 * 3

	; restore ecx
	pop		ecx

	; advance and save pointers
	add		ecx, mod_AccelerationProcess_size
	mov		[esp + 12], edi
	mov		[esp + 8], ecx
	ret

; ========================================
; Colour
; ========================================

; nothing to do on spawn
mod_ColourSpawn:
.end:

mod_ColourProcess:
	; load colour flags
	mov		eax, [0xFFFFFFFF]	; filled upon compilation
.postmov:
	; load previous colour (colour distributions require this)
	movaps	xmm5, xmm1
.predistr:
	; distribution code goes here
.postdistr:
	; move distribution result to target register
	movaps xmm1, xmm5
.end:

mod_ColourMeasure:
	mov		eax, [esi + ptcMod_Colour.Distr]
	distr_size eax, ecx, edx, c
	add		ecx, (mod_ColourProcess.end - mod_ColourProcess)
	; also add the flags to the data length
	add		edx, 4
	xor		ebx, ebx
	ret

mod_ColourCompile:
	; prepare pointers
	mov		ecx, [esp + 8]
	mov		edi, [esp + 12]

	; save off ecx
	push	ecx

	; copy our pre-distribution code over
	push	dword (mod_ColourProcess.predistr - mod_ColourProcess)
	push	mod_ColourProcess
	push	ecx
	call	memcpy
	add		esp, 4 * 3

	; restore ecx
	pop		ecx
	; fill in the read address
	mov		[ecx + (mod_ColourProcess.postmov - mod_ColourProcess - 4)], edi
	; put the flags in the data buffer
	mov		edx, [esi + ptcMod_Colour.Flags]
	mov		[edi], edx
	; advance the pointers
	add		ecx, (mod_ColourProcess.predistr - mod_ColourProcess)
	add		edi, 4

	; compile distribution
	add		esi, ptcMod_Colour.Distr
	mov		eax, [esi]
	distr_compile eax, c
	sub		esi, ptcMod_Colour.Distr

	; save off ecx
	push	ecx

	; copy our post-distribution code over
	mov		edx, (mod_ColourProcess.end - mod_ColourProcess.postdistr)
	push	edx
	push	dword mod_ColourProcess.postdistr
	push	ecx
	call	memcpy
	add		esp, 4 * 3

	; restore ecx
	pop		ecx

	; advance and save pointers
	add		ecx, (mod_ColourProcess.end - mod_ColourProcess.postdistr)
	mov		[esp + 12], edi
	mov		[esp + 8], ecx
	ret

; ========================================
; Size
; ========================================

; nothing to do on spawn
mod_SizeSpawn:
.end:

mod_SizeProcess:
	; distribution code goes here
	; move distribution result to target register
	fldz
	fxch	st4, st0
	fstp	st0
	faddp	st4, st0
.end:
mod_SizeProcess_size	equ	(mod_SizeProcess.end - mod_SizeProcess)

mod_SizeMeasure:
	mov		eax, [esi + ptcMod_Size.Distr]
	distr_size eax, ebx, edx, s
	add		ebx, (mod_SizeSpawn.end - mod_SizeSpawn)
	xor		ecx, ecx
	ret

mod_SizeCompile:
	; prepare pointers
	mov		ecx, [esp + 8]
	mov		edi, [esp + 12]

	; compile distribution
	add		esi, ptcMod_Size.Distr
	mov		eax, [esi]
	distr_compile eax, s
	sub		esi, ptcMod_Size.Distr

	; save off ecx
	push	ecx

	; copy our code over
	mov		edx, mod_SizeProcess_size
	push	edx
	push	dword mod_SizeProcess
	push	ecx
	call	memcpy
	add		esp, 4 * 3

	; restore ecx
	pop		ecx

	; advance and save pointers
	add		ecx, mod_SizeProcess_size
	mov		[esp + 12], edi
	mov		[esp + 4], ecx
	ret
	ret

; ========================================
; Gravity
; ========================================

; nothing to do on spawn
mod_GravitySpawn:
.end:

mod_GravityProcess:
	; TODO
.end:

mod_GravityMeasure:
	xor	ebx, ebx
	xor	ecx, ecx
	xor	edx, edx
	ret

mod_GravityCompile:
	ret

%endif ; PTC_MODULES
