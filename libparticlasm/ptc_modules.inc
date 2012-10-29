; particlasm modules code
; Copyright (C) 2011-2012, Leszek Godlewski <lg@inequation.org>

%ifndef PTC_MODULES
%define PTC_MODULES
; declarations
%include "libparticlasm.inc"

; libc memcpy
extern memcpy

; Spawning and processing calling convention:
; __dx - particle active or not
; [__sp + sizeof(ptr_t)] - pointer to rand() (as a workaround)
; [__sp + 2 * sizeof(ptr_t)] - INV_RAND_MAX
; [__sp + 2 * sizeof(ptr_t) + sizeof(float)] - pointer to MASK_RGB
; [__sp + 3 * sizeof(ptr_t) + sizeof(float)] - pointer to MASK_ALPHA
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
; __si - pointer to module
; __bx - spawn code size in bytes returned
; __cx - processing code size in bytes returned
; __dx - data size in bytes returned

; Compilation calling convention:
; (__sp + sizeof(ptr_t)) - working pointer to spawn code buffer
; (__sp + 2 * sizeof(ptr_t)) - working pointer to processing code buffer
; (__sp + 3 * sizeof(ptr_t)) - working pointer to data buffer

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
	mov		__dx, [__bx + ptcEmitter.NumParticles]
	dec		__dx;
	mov		[__bx + ptcEmitter.NumParticles], __dx
	xor		__dx, __dx
.return:
	ret
.end:
mod_SimulatePre_size	equ	(mod_SimulatePre.end - mod_SimulatePre)

mod_SimulatePreCompile:
	mov		__ax, [__sp + 2 * sizeof(ptr_t)]
	push	mod_SimulatePre_size
%ifidn __BITS__,64
	lea		__dx, [rel mod_SimulatePre]
	push	__dx
%else
	push	mod_SimulatePre
%endif
	push	__ax
	call	memcpy wrt ..plt
	add		__sp, 3 * sizeof(ptr_t)
	add		__ax, mod_SimulatePre_size
	mov		[__sp + 2 * sizeof(ptr_t)], __ax
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
	mov		__di, [__sp + 2 * sizeof(ptr_t)]
	; copy in the postprocessing code
	push	dword mod_SimulatePost_size
%ifidn __BITS__,64
	lea		__ax, [rel mod_SimulatePost]
	push	__ax
%else
	push	dword mod_SimulatePost
%endif
	push	__di
	call	memcpy wrt ..plt
	add		__sp, 3 * sizeof(ptr_t)
	add		__di, mod_SimulatePost_size
	mov		[__sp + 2 * sizeof(ptr_t)], __di

	; also drop in a ret instruction at the end of spawn code
	mov		__di, [__sp + sizeof(ptr_t)]
%ifidn __BITS__,64
	lea		__dx, [rel mod_SimulatePostCompile.ret_instruction]
	mov		dl, [__dx]
%else
	mov		dl, [mod_SimulatePostCompile.ret_instruction]
%endif
	mov		[__di], byte dl
	inc		__di
	mov		[__sp + sizeof(ptr_t)], __di
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
	mov		__ax, [__si + ptcMod_InitialLocation.Distr]
	distr_size __ax, __bx, __dx, v
	add		__bx, (mod_InitialLocationSpawn.end - mod_InitialLocationSpawn)
	xor		__cx, __cx
	ret

mod_InitialLocationCompile:
	; prepare pointers
	mov		__cx, [__sp + sizeof(ptr_t)]
	mov		__di, [__sp + 3 * sizeof(ptr_t)]

	; compile distribution
	add		__si, ptcMod_InitialLocation.Distr
	mov		__ax, [__si]
	distr_compile __ax, v
	sub		__si, ptcMod_InitialLocation.Distr

	; save off __cx
	push	__cx

	; copy our code over
	mov		__dx, mod_InitialLocationSpawn_size
	push	__dx
%ifidn __BITS__,64
	lea		__ax, [rel mod_InitialLocationSpawn]
	push	__ax
%else
	push	mod_InitialLocationSpawn
%endif
	push	__cx
	call	memcpy wrt ..plt
	add		__sp, 3 * sizeof(ptr_t)

	; restore __cx
	pop		__cx

	; advance and save pointers
	add		__cx, mod_InitialLocationSpawn_size
	mov		[__sp + 3 * sizeof(ptr_t)], __di
	mov		[__sp + sizeof(ptr_t)], __cx
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
	mov		__ax, [__si + ptcMod_InitialRotation.Distr]
	distr_size __ax, __bx, __dx, s
	add		__bx, (mod_InitialRotationSpawn.end - mod_InitialRotationSpawn)
	xor		__cx, __cx
	ret

mod_InitialRotationCompile:
	; prepare pointers
	mov		__cx, [__sp + sizeof(ptr_t)]
	mov		__di, [__sp + 3 * sizeof(ptr_t)]

	; compile distribution
	add		__si, ptcMod_InitialRotation.Distr
	mov		__ax, [__si]
	distr_compile __ax, s
	sub		__si, ptcMod_InitialRotation.Distr

	; save off __cx
	push	__cx

	; copy our code over
	mov		__dx, mod_InitialRotationSpawn_size
	push	__dx
%ifidn __BITS__,64
	lea		__ax, [rel mod_InitialRotationSpawn]
	push	__ax
%else
	push	ptr_t mod_InitialRotationSpawn
%endif
	push	__cx
	call	memcpy wrt ..plt
	add		__sp, 3 * sizeof(ptr_t)

	; restore __cx
	pop		__cx

	; advance and save pointers
	add		__cx, mod_InitialRotationSpawn_size
	mov		[__sp + 3 * sizeof(ptr_t)], __di
	mov		[__sp + sizeof(ptr_t)], __cx
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
	mov		__ax, [__si + ptcMod_InitialSize.Distr]
	distr_size __ax, __bx, __dx, s
	add		__bx, (mod_InitialSizeSpawn.end - mod_InitialSizeSpawn)
	xor		__cx, __cx
	ret

mod_InitialSizeCompile:
	; prepare pointers
	mov		__cx, [__sp + sizeof(ptr_t)]
	mov		__di, [__sp + 3 * sizeof(ptr_t)]

	; compile distribution
	add		__si, ptcMod_InitialSize.Distr
	mov		__ax, [__si]
	distr_compile __ax, s
	sub		__si, ptcMod_InitialSize.Distr

	; save off __cx
	push	__cx

	; copy our code over
	mov		__dx, mod_InitialSizeSpawn_size
	push	__dx
%ifidn __BITS__,64
	lea		__ax, [rel mod_InitialSizeSpawn]
	push	__ax
%else
	push	dword mod_InitialSizeSpawn
%endif
	push	__cx
	call	memcpy wrt ..plt
	add		__sp, 3 * sizeof(ptr_t)

	; restore __cx
	pop		__cx

	; advance and save pointers
	add		__cx, mod_InitialSizeSpawn_size
	mov		[__sp + 3 * sizeof(ptr_t)], __di
	mov		[__sp + sizeof(ptr_t)], __cx
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
	mov		__ax, [__si + ptcMod_InitialVelocity.Distr]
	distr_size __ax, __bx, __dx, v
	add		__bx, mod_InitialVelocitySpawn_size
	xor		__cx, __cx
	ret

mod_InitialVelocityCompile:
	; prepare pointers
	mov		__cx, [__sp + sizeof(ptr_t)]
	mov		__di, [__sp + 3 * sizeof(ptr_t)]

	; compile distribution
	add		__si, ptcMod_InitialVelocity.Distr
	mov		__ax, [__si]
	distr_compile __ax, v
	sub		__si, ptcMod_InitialVelocity.Distr

	; save off __cx
	push	__cx

	; copy our code over
	mov		__dx, mod_InitialVelocitySpawn_size
	push	__dx
%ifidn __BITS__,64
	lea		__ax, [rel mod_InitialVelocitySpawn]
	push	__ax
%else
	push	dword mod_InitialVelocitySpawn
%endif
	push	__cx
	call	memcpy wrt ..plt
	add		__sp, 3 * sizeof(ptr_t)

	; restore __cx
	pop		__cx

	; advance and save pointers
	add		__cx, mod_InitialVelocitySpawn_size
	mov		[__sp + 3 * sizeof(ptr_t)], __di
	mov		[__sp + sizeof(ptr_t)], __cx
	ret

; ========================================
; InitialColour
; ========================================

mod_InitialColourSpawn:
	; load colour flags
	mov __ax, [0xBADADD]	; filled upon compilation
.pr__distr:
	; distribution code goes here
.postdistr:
	; move distribution result to target register
	movaps xmm1, xmm5
.end:

; nothing to process
mod_InitialColourProcess:
.end:

mod_InitialColourMeasure:
	mov		__ax, [__si + ptcMod_InitialColour.Distr]
	distr_size __ax, __bx, __dx, c
	add		__bx, (mod_InitialColourSpawn.end - mod_InitialColourSpawn)
	; also add the flags to the data length
	add		__dx, 4
	xor		__cx, __cx
	ret

mod_InitialColourCompile:
	; prepare pointers
	mov		__cx, [__sp + sizeof(ptr_t)]
	mov		__di, [__sp + 3 * sizeof(ptr_t)]

	; save off __cx
	push	__cx

	; copy our pre-distribution code over
	push	dword (mod_InitialColourSpawn.pr__distr - mod_InitialColourSpawn)
%ifidn __BITS__,64
	lea		__ax, [rel mod_InitialColourSpawn]
	push	__ax
%else
	push	mod_InitialColourSpawn
%endif
	push	__cx
	call	memcpy wrt ..plt
	add		__sp, 3 * sizeof(ptr_t)

	; restore __cx
	pop		__cx
	; fill in the read address
	mov		[__cx + (mod_InitialColourSpawn.pr__distr - mod_InitialColourSpawn - 4)], __di
	; put the fake flags in the data buffer
	mov		[__di], dword (ptcCF_SetAlpha | ptcCF_SetRGB)
	; advance the pointers
	add		__cx, (mod_InitialColourSpawn.pr__distr - mod_InitialColourSpawn)
	add		__di, 4

	; compile distribution
	add		__si, ptcMod_InitialColour.Distr
	mov		__ax, [__si]
	distr_compile __ax, c
	sub		__si, ptcMod_InitialColour.Distr

	; save off __cx
	push	__cx

	; copy our post-distribution code over
	mov		__dx, (mod_InitialColourSpawn.end - mod_InitialColourSpawn.postdistr)
	push	__dx
%ifidn __BITS__,64
	lea		__ax, [rel mod_InitialColourSpawn.postdistr]
	push	__ax
%else
	push	mod_InitialColourSpawn.postdistr
%endif
	push	__cx
	call	memcpy wrt ..plt
	add		__sp, 3 * sizeof(ptr_t)

	; restore __cx
	pop		__cx

	; advance and save pointers
	add		__cx, (mod_InitialColourSpawn.end - mod_InitialColourSpawn.postdistr)
	mov		[__sp + 3 * sizeof(ptr_t)], __di
	mov		[__sp + sizeof(ptr_t)], __cx
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
	mov		__ax, [__si + ptcMod_Velocity.Distr]
	distr_size __ax, __cx, __dx, v
	add		__cx, (mod_VelocityProcess.end - mod_VelocityProcess)
	xor		__bx, __bx
	ret

mod_VelocityCompile:
	; prepare pointers
	mov		__cx, [__sp + 2 * sizeof(ptr_t)]
	mov		__di, [__sp + 3 * sizeof(ptr_t)]

	; compile distribution
	add		__si, ptcMod_Velocity.Distr
	mov		__ax, [__si]
	distr_compile __ax, v
	sub		__si, ptcMod_Velocity.Distr

	; save off __cx
	push	__cx

	; copy our code over
	mov		__dx, mod_VelocityProcess_size
	push	__dx
%ifidn __BITS__,64
	lea		__ax, [rel mod_VelocityProcess]
	push	__ax
%else
	push	mod_VelocityProcess
%endif
	push	__cx
	call	memcpy wrt ..plt
	add		__sp, 3 * sizeof(ptr_t)

	; restore __cx
	pop		__cx

	; advance and save pointers
	add		__cx, mod_VelocityProcess_size
	mov		[__sp + 3 * sizeof(ptr_t)], __di
	mov		[__sp + 2 * sizeof(ptr_t)], __cx
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
	mov __ax, [__si + ptcMod_Acceleration.Distr]
	distr_size __ax, __cx, __dx, v
	add __cx, (mod_AccelerationProcess.end - mod_AccelerationProcess)
	xor	__bx, __bx
	ret

mod_AccelerationCompile:
	; prepare pointers
	mov		__cx, [__sp + 2 * sizeof(ptr_t)]
	mov		__di, [__sp + 3 * sizeof(ptr_t)]

	; compile distribution
	add		__si, ptcMod_Acceleration.Distr
	mov		__ax, [__si]
	distr_compile __ax, v
	sub		__si, ptcMod_Acceleration.Distr

	; save off __cx
	push	__cx

	; copy our code over
	mov		__dx, mod_AccelerationProcess_size
	push	__dx
%ifidn __BITS__,64
	lea		__ax, [rel mod_AccelerationProcess]
	push	__ax
%else
	push	mod_AccelerationProcess
%endif
	push	__cx
	call	memcpy wrt ..plt
	add		__sp, 3 * sizeof(ptr_t)

	; restore __cx
	pop		__cx

	; advance and save pointers
	add		__cx, mod_AccelerationProcess_size
	mov		[__sp + 3 * sizeof(ptr_t)], __di
	mov		[__sp + 2 * sizeof(ptr_t)], __cx
	ret

; ========================================
; Colour
; ========================================

; nothing to do on spawn
mod_ColourSpawn:
.end:

mod_ColourProcess:
	; load colour flags
	mov		__ax, [0xBADADD]	; filled upon compilation
.postmov:
	; load previous colour (colour distributions require this)
	movaps	xmm5, xmm1
.pr__distr:
	; distribution code goes here
.postdistr:
	; move distribution result to target register
	movaps xmm1, xmm5
.end:

mod_ColourMeasure:
	mov		__ax, [__si + ptcMod_Colour.Distr]
	distr_size __ax, __cx, __dx, c
	add		__cx, (mod_ColourProcess.end - mod_ColourProcess)
	; also add the flags to the data length
	add		__dx, 4
	xor		__bx, __bx
	ret

mod_ColourCompile:
	; prepare pointers
	mov		__cx, [__sp + 2 * sizeof(ptr_t)]
	mov		__di, [__sp + 3 * sizeof(ptr_t)]

	; save off __cx
	push	__cx

	; copy our pre-distribution code over
	push	dword (mod_ColourProcess.pr__distr - mod_ColourProcess)
%ifidn __BITS__,64
	lea		__ax, [rel mod_ColourProcess]
	push	__ax
%else
	push	mod_ColourProcess
%endif
	push	__cx
	call	memcpy wrt ..plt
	add		__sp, 3 * sizeof(ptr_t)

	; restore __cx
	pop		__cx
	; fill in the read address
	mov		[__cx + (mod_ColourProcess.postmov - mod_ColourProcess - 4)], __di
	; put the flags in the data buffer
	mov		__dx, [__si + ptcMod_Colour.Flags]
	mov		[__di], __dx
	; advance the pointers
	add		__cx, (mod_ColourProcess.pr__distr - mod_ColourProcess)
	add		__di, 4

	; compile distribution
	add		__si, ptcMod_Colour.Distr
	mov		__ax, [__si]
	distr_compile __ax, c
	sub		__si, ptcMod_Colour.Distr

	; save off __cx
	push	__cx

	; copy our post-distribution code over
	mov		__dx, (mod_ColourProcess.end - mod_ColourProcess.postdistr)
	push	__dx
%ifidn __BITS__,64
	lea		__ax, [rel mod_ColourProcess.postdistr]
	push	__ax
%else
	push	mod_ColourProcess.postdistr
%endif
	push	__cx
	call	memcpy wrt ..plt
	add		__sp, 3 * sizeof(ptr_t)

	; restore __cx
	pop		__cx

	; advance and save pointers
	add		__cx, (mod_ColourProcess.end - mod_ColourProcess.postdistr)
	mov		[__sp + 3 * sizeof(ptr_t)], __di
	mov		[__sp + 2 * sizeof(ptr_t)], __cx
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
	mov		__ax, [__si + ptcMod_Size.Distr]
	distr_size __ax, __bx, __dx, s
	add		__bx, (mod_SizeSpawn.end - mod_SizeSpawn)
	xor		__cx, __cx
	ret

mod_SizeCompile:
	; prepare pointers
	mov		__cx, [__sp + 2 * sizeof(ptr_t)]
	mov		__di, [__sp + 3 * sizeof(ptr_t)]

	; compile distribution
	add		__si, ptcMod_Size.Distr
	mov		__ax, [__si]
	distr_compile __ax, s
	sub		__si, ptcMod_Size.Distr

	; save off __cx
	push	__cx

	; copy our code over
	mov		__dx, mod_SizeProcess_size
	push	__dx
%ifidn __BITS__,64
	lea		__ax, [rel mod_SizeProcess]
	push	__ax
%else
	push	mod_SizeProcess
%endif
	push	__cx
	call	memcpy wrt ..plt
	add		__sp, 3 * sizeof(ptr_t)

	; restore __cx
	pop		__cx

	; advance and save pointers
	add		__cx, mod_SizeProcess_size
	mov		[__sp + 3 * sizeof(ptr_t)], __di
	mov		[__sp + sizeof(ptr_t)], __cx
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
	xor	__bx, __bx
	xor	__cx, __cx
	xor	__dx, __dx
	ret

mod_GravityCompile:
	ret

%endif ; PTC_MODULES
