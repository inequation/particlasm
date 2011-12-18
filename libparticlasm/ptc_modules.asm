; particlasm modules code
; Copyright (C) 2011, Leszek Godlewski <lg@inequation.org>

%ifndef PTC_MODULES
%define PTC_MODULES
; declarations
%include "libparticlasm.inc"

; register assignments:
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

mod_InitialLocationSpawn:
	nop
.end:
	nop
mod_InitialLocationSpawn_size	equ	(mod_InitialLocationSpawn.end - mod_InitialLocationSpawn + 1)

mod_InitialLocationProcess:
	nop
.end:
	nop
mod_InitialLocationProcess_size	equ	(mod_InitialLocationProcess.end - mod_InitialLocationProcess + 1)

mod_InitialRotationSpawn:
	nop
.end:
	nop
mod_InitialRotationSpawn_size	equ	(mod_InitialRotationSpawn.end - mod_InitialRotationSpawn + 1)

mod_InitialRotationProcess:
	nop
.end:
	nop
mod_InitialRotationProcess_size	equ	(mod_InitialRotationProcess.end - mod_InitialRotationProcess + 1)

mod_InitialSizeSpawn:
	nop
.end:
	nop
mod_InitialSizeSpawn_size	equ	(mod_InitialSizeSpawn.end - mod_InitialSizeSpawn + 1)

mod_InitialSizeProcess:
	nop
.end:
	nop
mod_InitialSizeProcess_size	equ	(mod_InitialSizeProcess.end - mod_InitialSizeProcess + 1)

mod_InitialVelocitySpawn:
	nop
.end:
	nop
mod_InitialVelocitySpawn_size	equ	(mod_InitialVelocitySpawn.end - mod_InitialVelocitySpawn + 1)

mod_InitialVelocityProcess:
	nop
.end:
	nop
mod_InitialVelocityProcess_size	equ	(mod_InitialVelocityProcess.end - mod_InitialVelocityProcess + 1)

mod_InitialColourSpawn:
	nop
.end:
	nop
mod_InitialColourSpawn_size	equ	(mod_InitialColourSpawn.end - mod_InitialColourSpawn + 1)

mod_InitialColourProcess:
	nop
.end:
	nop
mod_InitialColourProcess_size	equ	(mod_InitialColourProcess.end - mod_InitialColourProcess + 1)

mod_VelocitySpawn:
	nop
.end:
	nop
mod_VelocitySpawn_size	equ	(mod_VelocitySpawn.end - mod_VelocitySpawn + 1)

mod_VelocityProcess:
	nop
.end:
	nop
mod_VelocityProcess_size	equ	(mod_VelocityProcess.end - mod_VelocityProcess + 1)

mod_AccelerationSpawn:
	nop
.end:
	nop
mod_AccelerationSpawn_size	equ	(mod_AccelerationSpawn.end - mod_AccelerationSpawn + 1)

mod_AccelerationProcess:
	nop
.end:
	nop
mod_AccelerationProcess_size	equ	(mod_AccelerationProcess.end - mod_AccelerationProcess + 1)

mod_ColourSpawn:
	nop
.end:
	nop
mod_ColourSpawn_size	equ	(mod_ColourSpawn.end - mod_ColourSpawn + 1)

mod_ColourProcess:
	nop
.end:
	nop
mod_ColourProcess_size	equ	(mod_ColourProcess.end - mod_ColourProcess + 1)

mod_SizeSpawn:
	nop
.end:
	nop
mod_SizeSpawn_size	equ	(mod_SizeSpawn.end - mod_SizeSpawn + 1)

mod_SizeProcess:
	nop
.end:
	nop
mod_SizeProcess_size	equ	(mod_SizeProcess.end - mod_SizeProcess + 1)

mod_GravitySpawn:
	nop
.end:
	nop
mod_GravitySpawn_size	equ	(mod_GravitySpawn.end - mod_GravitySpawn + 1)

mod_GravityProcess:
	nop
.end:
	nop
mod_GravityProcess_size	equ	(mod_GravityProcess.end - mod_GravityProcess + 1)

%endif ; PTC_MODULES
