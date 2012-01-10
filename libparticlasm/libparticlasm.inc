; particlasm NASM declarations
; Copyright (C) 2011, Leszek Godlewski <lg@inequation.org>

; AUTOGENERATED - DO NOT MODIFY!!!
; If you need to change something, change the corresponding libparticlasm.h
; declaration and re-run gen_asm_decls.py.

%ifndef LIBPARTICLASM_INC
%define LIBPARTICLASM_INC

; enumerations

; ptcModuleID
ptcMID_InitialLocation		equ	0d
ptcMID_InitialRotation		equ	1d
ptcMID_InitialSize		equ	2d
ptcMID_InitialVelocity		equ	3d
ptcMID_InitialColour		equ	4d
ptcMID_Velocity		equ	5d
ptcMID_Acceleration		equ	6d
ptcMID_Colour		equ	7d
ptcMID_Size		equ	8d
ptcMID_Gravity		equ	9d

; ptcDistributionID
ptcDID_Constant		equ	0d
ptcDID_Uniform		equ	1d
ptcDID_BicubicInterp		equ	2d

; ptcColourFlags
ptcCF_SetRGB		equ	1d
ptcCF_SetAlpha		equ	2d

; ptcGravityFlags
ptcGF_AxisX		equ	1d
ptcGF_AxisY		equ	2d
ptcGF_AxisZ		equ	4d
ptcGF_LinearAtt		equ	8d

; structures

struc ptcSDistr_Constant
	.DistrID:		resd	1
	.Val:		resd	1
endstruc

struc ptcSDistr_Uniform
	.DistrID:		resd	1
	.Range:		resd	2
endstruc

struc ptcSDistr_BicubicInterp
	.DistrID:		resd	1
	.TargVal:		resd	1
endstruc

struc ptcVDistr_Constant
	.DistrID:		resd	1
	.Val:		resd	3
endstruc

struc ptcVDistr_Uniform
	.DistrID:		resd	1
	.Ranges:		resd	6
endstruc

struc ptcVDistr_BicubicInterp
	.DistrID:		resd	1
	.TargVal:		resd	3
endstruc

struc ptcCDistr_Constant
	.DistrID:		resd	1
	.Val:		resd	4
endstruc

struc ptcCDistr_Uniform
	.DistrID:		resd	1
	.Ranges:		resd	8
endstruc

struc ptcCDistr_BicubicInterp
	.DistrID:		resd	1
	.TargVal:		resd	4
endstruc

struc ptcModuleHeader
	.ModuleID:		resd	1
	.Next:		resd	1
endstruc

struc ptcMod_InitialLocation
	.Header:		resb	ptcModuleHeader_size
	.Distr:		resb	28
endstruc

struc ptcMod_InitialRotation
	.Header:		resb	ptcModuleHeader_size
	.Distr:		resb	12
endstruc

struc ptcMod_InitialSize
	.Header:		resb	ptcModuleHeader_size
	.Distr:		resb	12
endstruc

struc ptcMod_InitialVelocity
	.Header:		resb	ptcModuleHeader_size
	.Distr:		resb	28
endstruc

struc ptcMod_InitialColour
	.Header:		resb	ptcModuleHeader_size
	.Distr:		resb	36
endstruc

struc ptcMod_Velocity
	.Header:		resb	ptcModuleHeader_size
	.Distr:		resb	28
endstruc

struc ptcMod_Acceleration
	.Header:		resb	ptcModuleHeader_size
	.Distr:		resb	28
endstruc

struc ptcMod_Colour
	.Header:		resb	ptcModuleHeader_size
	.Distr:		resb	36
	.Flags:		resd	1
endstruc

struc ptcMod_Size
	.Header:		resb	ptcModuleHeader_size
	.Distr:		resb	12
endstruc

struc ptcMod_Gravity
	.Header:		resb	ptcModuleHeader_size
	.Centre:		resb	28
	.Radius:		resd	1
	.SourceMass:		resd	1
	.Flags:		resd	1
endstruc

struc ptcParticle
	.Active:		resd	1
	.TimeScale:		resd	1
	.Time:		resd	1
	.Colour:		resd	4
	.Location:		resd	3
	.Rotation:		resd	1
	.Size:		resd	1
	.Velocity:		resd	3
	.Accel:		resd	3
endstruc

struc ptcVertex
	.Colour:		resd	4
	.Location:		resd	3
	.TexCoords:		resw	2
endstruc

struc ptcEmitter
	.SpawnRate:		resb	12
	.BurstCount:		resb	12
	.Period:		resd	1
	.LifeTimeFixed:		resd	1
	.LifeTimeRandom:		resd	1
	.InternalPtr1:		resd	1
	.InternalPtr2:		resd	1
	.InternalPtr3:		resd	1
	.Head:		resd	1
	.ParticleBuf:		resd	1
	.NumParticles:		resd	1
	.MaxParticles:		resd	1
endstruc

%endif ; LIBPARTICLASM_INC