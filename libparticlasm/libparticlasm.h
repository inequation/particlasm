/**
particlasm API header
Copyright (C) 2011, Leszek Godlewski <lg@inequation.org>

This file is also used for automatic NASM declaration generation (see the
gen_asm_decls.py script).

\author Leszek Godlewski
*/

#ifndef LIBPARTICLASM_H
#define LIBPARTICLASM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

/// 3-dimensional float vector (XYZ).
typedef float		ptcVector[3];
/// 4-component linear colour (RGBA, in [0..1]).
typedef float		ptcColour[4];

/// \name Identifiers and enumerations
/// @{

/// Identifier type. Set to correct enumeration value.
/// \sa ptcModuleID
/// \sa ptcDistributionID
typedef uint32_t	ptcID;

/// Enumerations to distinguish between the module IDs.
enum ptcModuleID {
	ptcMID_InitialLocation	= 0,	///< \sa ptcMod_InitialLocation
	ptcMID_InitialRotation,			///< \sa ptcMod_InitialRotation
	ptcMID_InitialSize,				///< \sa ptcMod_InitialSize
	ptcMID_InitialVelocity,			///< \sa ptcMod_InitialVelocity
	ptcMID_InitialColour,			///< \sa ptcMod_InitialColour
	ptcMID_Velocity,				///< \sa ptcMod_Velocity
	ptcMID_Acceleration,			///< \sa ptcMod_Acceleration
	ptcMID_Colour,					///< \sa ptcMod_Colour
	ptcMID_Size,					///< \sa ptcMod_Size
	ptcMID_Gravity					///< \sa ptcMod_Gravity
};

/// Enumerations to describe variable distributions.
enum ptcDistributionID {
	ptcDID_Constant			= 0,	///< constant distribution
	ptcDID_Uniform,					///< random, uniform distribution
	ptcDID_BicubicInterp			///< distribution based on a bicubic curve-interpolated as a function of normalized life time (i.e. in the [0..1] range)
};

/// @}

// ============================================================================

/// \name Variable distributions
/// @{

/// Constant distribution parameters - scalar. \sa ptcDistributionID
typedef struct {
	ptcID	DistrID;		///< distribution ID, must always be the corresponding \a ptcDistributionID value!
	float	Val;			///< value
} ptcSDistr_Constant;

/// Uniform random distribution parameters - scalar. \sa ptcDistributionID
typedef struct {
	ptcID	DistrID;		///< distribution ID, must always be the corresponding \a ptcDistributionID value!
	float	Range[2];		///< range from which values will be drawn, Range[0] to Range[1], inclusive
} ptcSDistr_Uniform;

/// Bicubic interpolation distribution - scalar. \sa ptcDistributionID
typedef struct {
	ptcID	DistrID;		///< distribution ID, must always be the corresponding \a ptcDistributionID value!
	float	TargVal;		///< value to interpolate to (from initial)
} ptcSDistr_BicubicInterp;

/// A union encompassing all of the scalar distributions.
typedef union {
	ptcID					DistrID;	///< distribution ID, must always be the corresponding \a ptcDistributionID value!
	ptcSDistr_Constant		Constant;
	ptcSDistr_Uniform		Uniform;
	ptcSDistr_BicubicInterp	BicubicInterp;
} ptcScalarDistr;

/// Constant distribution parameters - vector. \sa ptcDistributionID
typedef struct {
	ptcID		DistrID;	///< distribution ID, must always be the corresponding \a ptcDistributionID value!
	ptcVector	Val;		///< value
} ptcVDistr_Constant;

/// Uniform random distribution parameters - vector. \sa ptcDistributionID
typedef struct {
	ptcID		DistrID;	///< distribution ID, must always be the corresponding \a ptcDistributionID value!
	ptcVector	Ranges[2];	///< ranges from which values will be drawn, Ranges[0] to Ranges[1], inclusive
} ptcVDistr_Uniform;

/// Bicubic interpolation distribution - vector. \sa ptcDistributionID
typedef struct {
	ptcID		DistrID;	///< distribution ID, must always be the corresponding \a ptcDistributionID value!
	ptcVector	TargVal;	///< value to interpolate to (from initial)
} ptcVDistr_BicubicInterp;

/// An union encompassing all of the vector distributions.
typedef union {
	ptcID					DistrID;	///< distribution ID, must always be the corresponding \a ptcDistributionID value!
	ptcVDistr_Constant		Constant;
	ptcVDistr_Uniform		Uniform;
	ptcVDistr_BicubicInterp	BicubicInterp;
} ptcVectorDistr;

/// Constant distribution parameters - colour. \sa ptcDistributionID
typedef struct {
	ptcID		DistrID;	///< distribution ID, must always be the corresponding \a ptcDistributionID value!
	ptcColour	Val;		///< value
} ptcCDistr_Constant;

/// Uniform random distribution parameters - colour. \sa ptcDistributionID
typedef struct {
	ptcID		DistrID;	///< distribution ID, must always be the corresponding \a ptcDistributionID value!
	ptcColour	Ranges[2];	///< ranges from which values will be drawn, Ranges[0] to Ranges[1], inclusive
} ptcCDistr_Uniform;

/// Bicubic interpolation distribution - colour. \sa ptcDistributionID
typedef struct {
	ptcID		DistrID;	///< distribution ID, must always be the corresponding \a ptcDistributionID value!
	ptcColour	TargVal;	///< value to interpolate to (from initial)
} ptcCDistr_BicubicInterp;

/// An union encompassing all of the colour distributions.
typedef union {
	ptcID					DistrID;	///< distribution ID, must always be the corresponding \a ptcDistributionID value!
	ptcCDistr_Constant		Constant;
	ptcCDistr_Uniform		Uniform;
	ptcCDistr_BicubicInterp	BicubicInterp;
} ptcColourDistr;

/// @}

// ============================================================================

/// \name Module declarations
/// @{

/// Utility module pointer.
typedef	union ptcModule_u	*ptcModulePtr;

/// Module header struct.
typedef struct {
	ptcID				ModuleID;		///< module ID, must always be the corresponding \a ptcModuleID value!
	ptcModulePtr		Next;	///< pointer to the next module down the chain (NULL means last link)
} ptcModuleHeader;

/// Sets initial location of the particle.
typedef struct {
	ptcModuleHeader	Header;		///< module header
	ptcVectorDistr	Distr;		///< value distribution
} ptcMod_InitialLocation;

/// Sets initial rotation of the particle in radians.
typedef struct {
	ptcModuleHeader	Header;		///< module header
	ptcScalarDistr	Distr;		///< value distribution
} ptcMod_InitialRotation;

/// Sets initial size of the particle.
typedef struct {
	ptcModuleHeader	Header;		///< module header
	ptcScalarDistr	Distr;		///< value distribution
} ptcMod_InitialSize;

/// Sets initial velocity of the particle.
typedef struct {
	ptcModuleHeader	Header;		///< module header
	ptcVectorDistr	Distr;		///< value distribution
} ptcMod_InitialVelocity;

/// Sets initial colour of the particle.
typedef struct {
	ptcModuleHeader	Header;		///< module header
	ptcColourDistr	Distr;		///< value distribution
} ptcMod_InitialColour;

/// Processes velocity over the particle's life.
typedef struct {
	ptcModuleHeader	Header;		///< module header
	ptcVectorDistr	Distr;		///< value distribution
} ptcMod_Velocity;

/// Processes acceleration over the particle's life.
typedef struct {
	ptcModuleHeader	Header;		///< module header
	ptcVectorDistr	Distr;		///< value distribution
} ptcMod_Acceleration;

/// Colour flags to use with \a ptcMod_Colour.
enum ptcColourFlags {
	ptcCF_SetRGB	=	1,		///< if set, apply changes to the RGB components
	ptcCF_SetAlpha	=	2		///< if set, apply changes to alpha
};

/// Processes colour over the particle's life.
typedef struct {
	ptcModuleHeader	Header;		///< module header
	ptcColourDistr	Distr;		///< value distribution
	uint32_t		Flags;		///< colour flags \sa ptcColourFlags
} ptcMod_Colour;

/// Processes size over the particle's life.
typedef struct {
	ptcModuleHeader	Header;		///< module header
	ptcScalarDistr	Distr;		///< value distribution
} ptcMod_Size;

/// Gravity flags to use with \a ptcMod_Gravity
enum ptcGravityFlags {
	ptcGF_AxisX		=	1,		///< gravity acts on the X axis
	ptcGF_AxisY		=	2,		///< gravity acts on the Y axis
	ptcGF_AxisZ		=	4,		///< gravity acts on the X axis
	ptcGF_LinearAtt	=	8,		///< linear attenuation instead of quadratic
};

/// Makes all of this emitter's particles gravitate towards a given point using
/// Newton's law of universal gravitation equation. Particle mass is assumed to
/// be 1 to simplify the calculations.
typedef struct {
	ptcModuleHeader	Header;		///< module header
	ptcVectorDistr	Centre;		///< location of gravity centre
	float			Radius;		///< gravity source radius (linear falloff inside)
	float			SourceMass;	///< mass of the gravity source
	uint32_t		Flags;		///< gravity flags \sa ptcGravityFlags
} ptcMod_Gravity;

/// Common interface to all the modules.
typedef union ptcModule_u {
	ptcModuleHeader			Header;
	ptcMod_InitialLocation	InitLoc;
	ptcMod_InitialRotation	InitRot;
	ptcMod_InitialSize		InitSize;
	ptcMod_InitialVelocity	InitVel;
	ptcMod_InitialColour	InitCol;
	ptcMod_Velocity			Vel;
	ptcMod_Acceleration		Accel;
	ptcMod_Colour			Col;
	ptcMod_Size				Size;
	ptcMod_Gravity			Gravity;
} ptcModule;

/// @}

/// Single particle structure.
typedef struct {
	uint32_t	Active;			///< zero - particle is inactive, non-zero - active
	float		TimeScale;		///< multiplier by which simulation step time is scaled for this particle (effectively controls life time)
	float		Time;			///< normalized particle life time (i.e. in the range [0..1])
	ptcColour	Colour;			///< particle colour
	ptcVector	Location;		///< location of the particle
	float		Rotation;		///< rotation of the particle (radians)
	float		Size;			///< particle size (i.e. length of the square's side)
	ptcVector	Velocity;		///< particle's velocity
	ptcVector	Accel;			///< particle's acceleration
} ptcParticle;

/// Particle vertex structure.
typedef struct {
	ptcColour	Colour;			///< vertex colour
	ptcVector	Location;		///< vertex location
	int16_t		TexCoords[2];	///< texture coordinates
} ptcVertex;

/// Entire emitter configuration.
typedef struct {
	float			SpawnRate;		///< particle spawn rate in bursts per second
	uint32_t		BurstCount;		///< number of particles per burst
	float			Period;			///< length of full emitter period (for scaling the effects of \a SpawnRate and \a BurstCount in time)
	float			LifeTimeFixed;	///< life time of a particle (fixed part)
	float			LifeTimeRandom;	///< life time of a particle (random part)
	void			*InternalPtr4;	///< pointer to internal particlasm data structure
	void			*InternalPtr1;	///< pointer to internal particlasm data structure
	void			*InternalPtr2;	///< pointer to internal particlasm data structure
	void			*InternalPtr3;	///< pointer to internal particlasm data structure
	ptcModule		*Head;			///< pointer to first module
	ptcParticle		*ParticleBuf;	///< pointer to particle buffer
	uint32_t		NumParticles;	///< current number of particles
	uint32_t		MaxParticles;	///< maximum number of particles (size of the particle buffer)
} ptcEmitter;

/// Function attribute declaration - here, we're explicitly declaring the
/// calling convention as cdecl with 16-byte stack alignment.
#ifdef __GNUC__
	#define PTC_ATTRIBS	__attribute__((cdecl))
#else
	#define PTC_ATTRIBS	__declspec(cdecl)
#endif // __GNUC__

/// Compiles a particle emitter given the emitter settings. Sets
/// emitter->InternalPtr.
/// \param	emitter				emitter to compile
/// \return	non-zero on success, zero on failure
typedef PTC_ATTRIBS uint32_t (* PFNPTCCOMPILEEMITTER)(ptcEmitter *emitter);

/// Spawns new particles according to the given emitter's parameters and
/// advances all the existing ones by a simulation step, emitting particle
/// vertices to the given buffer.
/// \param	emitter		pointer to the emitter in question
/// \param	step		simulation time step
/// \param	cameraCS	camera coordinate system (forward, right, up vectors)
/// \param	buffer		pointer to the vertex buffer to emit particle vertices to
/// \param	maxVertices	maximum number of vertices to emit
/// \return	number of particle vertices emitted
typedef PTC_ATTRIBS uint32_t (* PFNPTCPROCESSEMITTER)(ptcEmitter *emitter,
	float step, ptcVector cameraCS[3], ptcVertex *buffer, uint32_t maxVertices);

/// Releases all resources related to this emitter.
/// \param	emitter				emitter to release
typedef PTC_ATTRIBS void (* PFNPTCRELEASEEMITTER)(ptcEmitter *emitter);

#ifdef __cplusplus
}
#endif

#endif // LIBPARTICLASM_H
