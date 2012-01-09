; particlasm modules code
; Copyright (C) 2011, Leszek Godlewski <lg@inequation.org>

%ifndef PTC_MODULES
%define PTC_MODULES
; declarations
%include "libparticlasm.inc"

; Spawning and processing calling convention:
; edx - particle active or not
; ebx - pointer to the emitter
;
; st0 - simulation step
; st1 - ptcParticle.TimeScale
; st2 - ptcParticle.Time
; st3 - ptcParticle.Rotation
; st4 - ptcParticle.Size
;
; xmm0 - simulation step (as a vector)
; xmm1 - ptcParticle.Colour
; xmm2 - ptcParticle.Location
; xmm3 - ptcParticle.Velocity
; xmm4 - ptcParticle.Accel

; Measurement calling convention:
; eax - pointer to module
; ebx - spawn code size in bytes returned
; ecx - processing code size in bytes returned
; edx - data size in bytes returned

; Compilation calling convention:
; (ebp - 4) - pointer to spawn code buffer
; (ebp - 8) - pointer to processing code buffer
; (ebp - 12) - pointer to data buffer

; ========================================
; InitialLocation
; ========================================

mod_InitialLocationSpawn:
	ret
.end:

mod_InitialLocationProcess:
	ret
.end:

mod_InitialLocationMeasure:
	mov	ebx, 0
	mov	ecx, 0
	mov	edx, 0
	ret

mod_InitialLocationCompile:
	ret

; ========================================
; InitialRotation
; ========================================

mod_InitialRotationSpawn:
	ret
.end:

mod_InitialRotationProcess:
	ret
.end:

mod_InitialRotationMeasure:
	mov	ebx, 0
	mov	ecx, 0
	mov	edx, 0
	ret

mod_InitialRotationCompile:
	ret

; ========================================
; InitialSize
; ========================================

mod_InitialSizeSpawn:
	ret
.end:

mod_InitialSizeProcess:
	ret
.end:

mod_InitialSizeMeasure:
	mov	ebx, 0
	mov	ecx, 0
	mov	edx, 0
	ret

mod_InitialSizeCompile:
	ret

; ========================================
; InitialVelocity
; ========================================

mod_InitialVelocitySpawn:
	ret
.end:

mod_InitialVelocityProcess:
	ret
.end:

mod_InitialVelocityMeasure:
	mov	ebx, 0
	mov	ecx, 0
	mov	edx, 0
	ret

mod_InitialVelocityCompile:
	ret

; ========================================
; InitialColour
; ========================================

mod_InitialColourSpawn:
	ret
.end:

mod_InitialColourProcess:
	ret
.end:

mod_InitialColourMeasure:
	mov	ebx, 0
	mov	ecx, 0
	mov	edx, 0
	ret

mod_InitialColourCompile:
	ret

; ========================================
; Velocity
; ========================================

mod_VelocitySpawn:
	ret
.end:

mod_VelocityProcess:
	ret
.end:

mod_VelocityMeasure:
	mov	ebx, 0
	mov	ecx, 0
	mov	edx, 0
	ret

mod_VelocityCompile:
	ret

; ========================================
; Acceleration
; ========================================

mod_AccelerationSpawn:
	ret
.end:

mod_AccelerationProcess:
	ret
.end:

mod_AccelerationMeasure:
	mov	ebx, 0
	mov	ecx, 0
	mov	edx, 0
	ret

mod_AccelerationCompile:
	ret

; ========================================
; Colour
; ========================================

mod_ColourSpawn:
	ret
.end:

mod_ColourProcess:
	ret
.end:

mod_ColourMeasure:
	mov	ebx, 0
	mov	ecx, 0
	mov	edx, 0
	ret

mod_ColourCompile:
	ret

; ========================================
; Size
; ========================================

mod_SizeSpawn:
	ret
.end:

mod_SizeProcess:
	ret
.end:

mod_SizeMeasure:
	mov	ebx, 0
	mov	ecx, 0
	mov	edx, 0
	ret

mod_SizeCompile:
	ret

; ========================================
; Gravity
; ========================================

mod_GravitySpawn:
	ret
.end:

mod_GravityProcess:
	ret
.end:

mod_GravityMeasure:
	mov	ebx, 0
	mov	ecx, 0
	mov	edx, 0
	ret

mod_GravityCompile:
	ret

%endif ; PTC_MODULES
