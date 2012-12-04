/**
Particlasm 2
libparticlasm2 API header
Copyright (C) 2011-2012, Leszek Godlewski <github@inequation.org>

This file is also used for automatic generation of target-specific declarations
(for an example, see the \a GenerateAsmInclude.py script in the X86Assembly
target source tree).

\author Leszek Godlewski
*/

#ifndef LIBPARTICLASM2_H
#define LIBPARTICLASM2_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stddef.h>

/// Floating point scalar.
typedef float		ptcScalar;
/// 3-dimensional float vector (XYZ).
typedef ptcScalar	ptcVector[3];
/// 4-component linear colour (RGBA, in [0..1]).
typedef ptcScalar	ptcColour[4];

/// \name Identifiers and enumerations
/// @{

/// Identifier type. Set to correct enumeration value.
/// \sa ptcModuleID
/// \sa ptcDistributionID
typedef uint32_t	ptcID;

/// Enumeration to distinguish between the module IDs.
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

/// Enumeration to describe variable distributions.
enum ptcDistributionID {
	ptcDID_Constant			= 0,	///< constant distribution
	ptcDID_Uniform,					///< random, uniform distribution
	ptcDID_BicubicInterp			///< distribution based on a bicubic curve-interpolated as a function of normalized life time (i.e. in the [0..1] range)
};

/// Enumeration of supported target platforms.
enum ptcTarget {
	ptcTarget_RuntimeInterpreter,	///< runtime interpreter written in C++ (a.k.a. reference implementation)
	ptcTarget_x86,					///< x86 (i386) assembly
	ptcTarget_x86_64,				///< x86-64 (AMD64) assembly
};

/// @}

// ============================================================================

/// \name Variable distributions
/// @{

/// Constant distribution parameters - scalar. \sa ptcDistributionID
typedef struct {
	ptcID		DistrID;		///< distribution ID, must always be the corresponding \a ptcDistributionID value!
	ptcScalar	Val;			///< value
} ptcSDistr_Constant;

/// Uniform random distribution parameters - scalar. \sa ptcDistributionID
typedef struct {
	ptcID		DistrID;		///< distribution ID, must always be the corresponding \a ptcDistributionID value!
	ptcScalar	Range[2];		///< range from which values will be drawn, Range[0] to Range[1], inclusive
} ptcSDistr_Uniform;

/// Bicubic interpolation distribution - scalar. \sa ptcDistributionID
typedef struct {
	ptcID		DistrID;		///< distribution ID, must always be the corresponding \a ptcDistributionID value!
	ptcScalar	TargVal;		///< value to interpolate to (from initial)
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
	ptcID			ModuleID;	///< module ID, must always be the corresponding \a ptcModuleID value!
	ptcModulePtr	Next;		///< pointer to the next module down the chain (NULL means last link)
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
	ptcScalar		Radius;		///< gravity source radius (linear falloff inside)
	ptcScalar		SourceMass;	///< mass of the gravity source
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

// ============================================================================

/// \name Emitter-specific and runtime structures
/// @{

/// Individual run-time particle structure.
typedef struct {
	uint32_t	Active;			///< zero - particle is inactive, non-zero - active
	ptcScalar	TimeScale;		///< multiplier by which simulation step time is scaled for this particle (effectively controls life time)
	ptcScalar	Time;			///< normalized particle life time (i.e. in the range [0..1])
	ptcColour	Colour;			///< particle colour
	ptcVector	Location;		///< location of the particle
	ptcScalar	Rotation;		///< rotation of the particle (radians)
	ptcScalar	Size;			///< particle size (i.e. length of the square's side)
	ptcVector	Velocity;		///< particle's velocity
	ptcVector	Accel;			///< particle's acceleration
} ptcParticle;

/// Particle vertex structure.
typedef struct {
	ptcColour	Colour;			///< vertex colour
	ptcVector	Location;		///< vertex location
	int16_t		TexCoords[2];	///< texture coordinates
} ptcVertex;

/// Emitter configuration.
typedef struct {
	ptcScalar		SpawnRate;		///< particle spawn rate in bursts per second
	uint32_t		BurstCount;		///< number of particles per burst
	ptcScalar		LifeTimeFixed;	///< life time of a particle (fixed part)
	ptcScalar		LifeTimeRandom;	///< life time of a particle (random part)
} ptcEmitterConfig;

/// Entire run-time emitter structure.
typedef struct {
	ptcEmitterConfig	Config;			///< platform-independent emitter configuration

	ptcScalar			SpawnTimer;		///< working variable that keeps track of when to spawn new particles
	size_t				NumParticles;	///< current number of particles
	size_t				MaxParticles;	///< maximum number of particles (size of the particle buffer)

	// pointer size varies (32 or 64 bits) per platform
	void				*InternalPtr1;	///< pointer to internal particlasm data structure
	void				*InternalPtr2;	///< pointer to internal particlasm data structure
	void				*InternalPtr3;	///< pointer to internal particlasm data structure
	ptcModule			*Head;			///< pointer to first module
	ptcParticle			*ParticleBuf;	///< pointer to particle buffer
} ptcEmitter;

/// @}

// ============================================================================

/// \name Entry point and APIs
/// @{

/// Function attribute declaration - here, we're explicitly declaring the
/// calling convention as cdecl (for 32-bit platforms; there is only one
/// convention on x86-64) with 16-byte stack alignment.
#ifdef __GNUC__
       #if __x86_64__
               #define PTC_ATTRIBS
       #else
               #define PTC_ATTRIBS     __attribute__((cdecl))
       #endif // __x86_64__
#else
       #if _WIN64
               #define PTC_ATTRIBS
       #else
               #define PTC_ATTRIBS     __declspec(cdecl)
       #endif // _WIN64
#endif // __GNUC__

typedef struct {
	/// Queries the library whether support for the given target platform has been
	/// compiled in.
	/// \param	target		a \a ptcTarget enumeration member
	/// \return non-zero if target is supported, zero if it's not
	/// \sa PFNPTCGETAPI
	PTC_ATTRIBS uint32_t (* QueryTargetSupport)(ptcTarget target);

	/// Initializes the given target.
	/// \param	target		a \a ptcTarget enumeration member
	/// \param	privateData	pointer to target-specific private data; refer to target documentation for details
	/// \return non-zero on success, zero on failure
	PTC_ATTRIBS uint32_t (* InitializeTarget)(ptcTarget target,
		void *privateData);

	/// Shuts down the given target.
	/// \param	privateData	pointer to target-specific private data; refer to target documentation for details
	PTC_ATTRIBS void (* ShutdownTarget)(void *privateData);

	/// Compiles a particle emitter given the emitter settings. Sets
	/// emitter->InternalPtr*.
	/// \param	emitter				emitter to compile
	/// \return	non-zero on success, zero on failure
	/// \sa PFNPTCGETAPI
	PTC_ATTRIBS uint32_t (* CompileEmitter)(ptcEmitter *emitter);

	/// Spawns new particles according to the given emitter's parameters and
	/// advances all the existing ones by a simulation step, emitting particle
	/// vertices to the given buffer.
	/// \param	emitter		pointer to the emitter in question
	/// \param	step		simulation time step
	/// \param	cameraCS	camera coordinate system (forward, right, up vectors)
	/// \param	buffer		pointer to the vertex buffer to emit particle vertices to
	/// \param	maxVertices	maximum number of vertices to emit
	/// \return	number of particle vertices emitted
	/// \sa PFNPTCGETAPI
	PTC_ATTRIBS uint32_t (* ProcessEmitter)(ptcEmitter *emitter,
		ptcScalar step, ptcVector cameraCS[3], ptcVertex *buffer,
		uint32_t maxVertices);

	/// Releases all resources related to this emitter.
	/// \param	emitter		emitter to release
	/// \sa PFNPTCGETAPI
	PTC_ATTRIBS void (* ReleaseEmitter)(ptcEmitter *emitter);
} ptcAPIExports;

/// Compiled-in version of libparticlasm API.
/// \sa PFNPTCGETAPI
#define PTC_API_VERSION		1

/// The actual entry point to the library.
/// \param	clientVersion	the \a PTC_API_VERSION the client has been compiled with
/// \param	API				pointer to a ptcAPIExports structure that gets filled with function addresses, if compiled and linked versions match; it is untouched otherwise
/// \return non-zero on success, zero on failure (compiled vs linked version mismatch)
/// \sa PTC_ENTRY_POINT
/// \sa PTC_API_VERSION
typedef PTC_ATTRIBS uint32_t (* PFNPTCGETAPI)(uint32_t clientVersion,
	ptcAPIExports *API);

/// Stringized name of the library's entry point. For convenience when using GetProcAddress() or dlsym().
/// \sa PTC_ENTRY_POINT_S
/// \sa PFNPTCGETAPI
#define PTC_ENTRY_POINT		"ptcGetAPI"

/// @}

#ifdef __cplusplus
}
#endif

#endif // LIBPARTICLASM2_H
