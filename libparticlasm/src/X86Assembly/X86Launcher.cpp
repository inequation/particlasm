/*
Particlasm compiled code launcher for X86 assembly
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include <cstdlib>
#include <cstring>
#include "../Core/ParticlasmMain.h"
#include "ExternalFuncLibrary.h"
#include "X86Launcher.h"

#if defined(WIN32) || defined(__WIN32__)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else
	#include <sys/types.h>
	#include <sys/mman.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <map>
	#include <unistd.h>
#endif // WIN32

// native word width register aliases
#if defined(_M_AMD64) || defined(amd64) || defined (__amd64__)
	#define __AX			"rax"
	#define __BX			"rbx"
	#define __CX			"rcx"
	#define __DX			"rdx"
	#define __SI			"rsi"
	#define __DI			"rdi"
	#define __BP			"rbp"
	#define __SP			"rsp"
	#define __CALL			"callq"
	// selective pushad/popad - leave rdx intact, it contains life state
#define __PUSHAD			"sub	$40, %%rsp\n"	\
							"movq	%%rax, (%%rsp)\n"	\
							"movq	%%rbx, 8(%%rsp)\n"	\
							"movq	%%rcx, 16(%%rsp)\n"	\
							"movq	%%rsi, 24(%%rsp)\n"	\
							"movq	%%rdi, 32(%%rsp)\n"
#define __POPAD				"movq	32(%%rsp), %%rdi\n"	\
							"movq	24(%%rsp), %%rsi\n"	\
							"movq	16(%%rsp), %%rcx\n"	\
							"movq	8(%%rsp), %%rbx\n"	\
							"movq	(%%rsp), %%rax\n"	\
							"add	$40, %%rsp\n"
#else
	#define __AX			"eax"
	#define __BX			"ebx"
	#define __CX			"ecx"
	#define __DX			"edx"
	#define __SI			"esi"
	#define __DI			"edi"
	#define __BP			"ebp"
	#define __SP			"esp"
	#define __CALL			"calll"
	// selective pushad/popad - leave edx intact, it contains life state
#define __PUSHAD			"sub	$20, %%esp\n"	\
							"movl	%%eax, (%%esp)\n"	\
							"movl	%%ebx, 4(%%esp)\n"	\
							"movl	%%ecx, 8(%%esp)\n"	\
							"movl	%%esi, 12(%%esp)\n"	\
							"movl	%%edi, 16(%%esp)\n"
#define __POPAD				"movl	16(%%esp), %%edi\n"	\
							"movl	12(%%esp), %%esi\n"	\
							"movl	8(%%esp), %%ecx\n"	\
							"movl	4(%%esp), %%ebx\n"	\
							"movl	(%%esp), %%eax\n"	\
							"add	$20, %%esp\n"
#endif

#define DECLARE_STRUCT_OFFSET(Type, Member)									\
	[Member] "i" (offsetof(Type, Member))

#define LOAD_PARTICLE_DATA(Particle, TimeStep)								\
	asm("mov	%c[Active](%0), %%edx\n"									\
		"fld	%c[Size](%0)\n"												\
		"fld	%c[Rotation](%0)\n"											\
		"fld	%c[Time](%0)\n"												\
		"fld	%c[TimeScale](%0)\n"										\
		"movups	%c[Colour](%0), %%xmm1\n"									\
		"movups	%c[Location](%0), %%xmm2\n"									\
		"movups	%c[Velocity](%0), %%xmm3\n"									\
		"movups	%c[Accel](%0), %%xmm4\n"									\
		"fld	%1\n"														\
		"movss	%1, %%xmm0\n"												\
		"shufps	$0x00, %%xmm0, %%xmm0\n"									\
		:																	\
		: "r" (Particle),													\
			"m" (TimeStep),													\
			DECLARE_STRUCT_OFFSET(ptcParticle, Active),						\
			DECLARE_STRUCT_OFFSET(ptcParticle, Size),						\
			DECLARE_STRUCT_OFFSET(ptcParticle, Rotation),					\
			DECLARE_STRUCT_OFFSET(ptcParticle, Time),						\
			DECLARE_STRUCT_OFFSET(ptcParticle, TimeScale),					\
			DECLARE_STRUCT_OFFSET(ptcParticle, Colour),						\
			DECLARE_STRUCT_OFFSET(ptcParticle, Location),					\
			DECLARE_STRUCT_OFFSET(ptcParticle, Velocity),					\
			DECLARE_STRUCT_OFFSET(ptcParticle, Accel)						\
		: "%edx", "%st", "%st(1)", "%st(2)", "%st(3)", "%st(4)", "%xmm0",	\
			"%xmm1", "%xmm2", "%xmm3", "%xmm4"								\
	)

/*
Our vectors are 12 bytes long, so these stores require some SSE magic: first
store the low order half of the register, then broadcast the 3rd float onto the
entire register and store it as a scalar at an offset.
Apart from colour, which is 16 bytes long, therefore it can be written directly.
*/
#define STORE_PARTICLE_DATA(Particle)										\
	asm("fstp	%%st(0)\n"													\
		"movlps	%%xmm4, %c[Accel](%0)\n"									\
		"shufps	$0xAA, %%xmm4, %%xmm4\n"									\
		"movss	%%xmm4, %c[Accel] + 8(%0)\n"								\
		"movlps	%%xmm3, %c[Velocity](%0)\n"									\
		"shufps	$0xAA, %%xmm3, %%xmm3\n"									\
		"movss	%%xmm3, %c[Velocity] + 8(%0)\n"								\
		"movlps	%%xmm2, %c[Location](%0)\n"									\
		"shufps	$0xAA, %%xmm2, %%xmm2\n"									\
		"movss	%%xmm2, %c[Location] + 8(%0)\n"								\
		"movups	%%xmm1, %c[Colour](%0)\n"									\
		"fstp	%c[TimeScale](%0)\n"										\
		"fstp	%c[Time](%0)\n"												\
		"fstp	%c[Rotation](%0)\n"											\
		"fstp	%c[Size](%0)\n"												\
		"mov	%%edx, %c[Active](%0)\n"									\
		:																	\
		: "r" (Particle),													\
			DECLARE_STRUCT_OFFSET(ptcParticle, Active),						\
			DECLARE_STRUCT_OFFSET(ptcParticle, Size),						\
			DECLARE_STRUCT_OFFSET(ptcParticle, Rotation),					\
			DECLARE_STRUCT_OFFSET(ptcParticle, Time),						\
			DECLARE_STRUCT_OFFSET(ptcParticle, TimeScale),					\
			DECLARE_STRUCT_OFFSET(ptcParticle, Colour),						\
			DECLARE_STRUCT_OFFSET(ptcParticle, Location),					\
			DECLARE_STRUCT_OFFSET(ptcParticle, Velocity),					\
			DECLARE_STRUCT_OFFSET(ptcParticle, Accel)						\
		: "%edx", "%st", "%st(1)", "%st(2)", "%st(3)", "%st(4)"				\
	)

#define CALL_ASSEMBLY(Pointer)												\
	asm volatile(															\
		__PUSHAD "\n"														\
		"push	%c[FRand](%0)\n"											\
		__CALL "	*%1\n"													\
		"add	%2, %%" __SP "\n"											\
		__POPAD "\n"														\
		:																	\
		: "r" (&ExtLib),													\
			"r" (Pointer),													\
			"i" (sizeof(ptcExtLib)),										\
			DECLARE_STRUCT_OFFSET(ptcExtLib, FRand)							\
		: "%edx", "%xmm0", "%xmm1", "%xmm2", "%xmm3",						\
			"%xmm4", "%xmm5", "%xmm6", "%xmm7",								\
			"%st", "%st(1)", "%st(2)", "%st(3)"								\
	)

namespace mt19937 {
	extern void init_by_array(unsigned long init_key[], int key_length);
	extern unsigned long genrand_int32(void);
}

static inline float FRand()
{
	uint32_t RandInt = mt19937::genrand_int32();
	static const float Multiplier = 1.0f / 4294967295.0f;
	return RandInt * Multiplier;
}

static void ExtLib_FRand(void)
{
	// The MT returns an unsigned 32-bit integer, while the FPU can only deal
	// with signed numbers; as a result, the unsigned 32-bit integer is
	// interpreted as a signed one, and we get a float in the [-0.5, 0.5] range
	// instead of [0, 1]. We can fix that by either throwing 1 bit away and
	// reducing the divisor by half, or by zero-expansion to 64 bits. I chose
	// the latter to preserve the granularity, and I presume this is also what
	// the compiler silently does in the C++ implementation above.
	int64_t RandInt = static_cast<int64_t>(mt19937::genrand_int32());
	static float Multiplier = 1.0f / 4294967295.0f;
	asm("flds	%0\n"
		"fildq	%1\n"
		"fmulp\n"
		: // no output registers
		: "m" (Multiplier), "m" (RandInt)
		: "%st", "%st(1)"
		);
}

static ptcExtLib ExtLib = {ExtLib_FRand};

X86Launcher::X86Launcher()
{
	// use the opportunity to initialize the mersenne twister
	unsigned long init[4] = {
		(unsigned long)rand(), (unsigned long)rand(),
		(unsigned long)rand(), (unsigned long)rand()
	}, length = 4;
	mt19937::init_by_array(init, length);
}

// get rid of unused parameter warnings
SUPPRESS_WARNING_GCC_BEGIN("-Wunused-parameter")

bool X86Launcher::LoadCachedBinary(ptcEmitter *Emitter, const void *Buffer,
	const size_t Size) const
{
	assert(!"Unimplemented " STRINGIFY(__FUNCTION__));
	return false;
}

size_t X86Launcher::WriteCachedBinary(ptcEmitter *Emitter,
	PFNWRITE WriteCallback) const
{
	assert(!"Unimplemented " STRINGIFY(__FUNCTION__));
	return 0;
}

bool X86Launcher::LoadRawBinary(ptcEmitter *Emitter, const void *Buffer,
	const size_t Size, const size_t DataOffset, const size_t SpawnCodeOffset,
	const size_t ProcessCodeOffset) const
{
	assert(DataOffset == 0);
	Emitter->InternalPtr1 = mmap(NULL, Size,
		PROT_READ | PROT_WRITE | PROT_EXEC,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (Emitter->InternalPtr1 == MAP_FAILED)
		return false;
	memcpy(Emitter->InternalPtr1, Buffer, Size);
	Emitter->InternalPtr2 = ((char *)Emitter->InternalPtr1) + SpawnCodeOffset;
	Emitter->InternalPtr3 = ((char *)Emitter->InternalPtr1) + ProcessCodeOffset;
	return true;
}

void X86Launcher::Unload(ptcEmitter *Emitter) const
{
	munmap(Emitter->InternalPtr1, 0);
}

void X86Launcher::SpawnParticles(ptcEmitter *Emitter, float TimeStep,
	uint32_t Count) const
{
	size_t Iterations = 0;
	// start at a random index to reduce chances of a collision
	for (uint32_t Index = mt19937::genrand_int32() % Emitter->MaxParticles;
		Count > 0 && Emitter->NumParticles < Emitter->MaxParticles
			&& Iterations < Emitter->MaxParticles;
		++Iterations, Index = (Index + 1) % Emitter->MaxParticles)
	{
		volatile ptcParticle *Particle = Emitter->ParticleBuf + Index;
		if (Particle->Active)
			continue;
		// increase the particle counter
		++Emitter->NumParticles;
		// clear particle data
		Particle->Active = 1;
		Particle->TimeScale = 1.f / (
			Emitter->Config.LifeTimeFixed
			+ FRand() * Emitter->Config.LifeTimeRandom);
		Particle->Time = 0.f;
		Particle->Colour[0] = 1.f;
		Particle->Colour[1] = 1.f;
		Particle->Colour[2] = 1.f;
		Particle->Colour[3] = 1.f;
		Particle->Location[0] = 0.f;
		Particle->Location[1] = 0.f;
		Particle->Location[2] = 0.f;
		Particle->Rotation = 0.f;
		Particle->Size = 1.f;
		Particle->Velocity[0] = 0.f;
		Particle->Velocity[1] = 0.f;
		Particle->Velocity[2] = 0.f;
		Particle->Accel[0] = 0.f;
		Particle->Accel[1] = 0.f;
		Particle->Accel[2] = 0.f;

		// we're ready to call assembly
		LOAD_PARTICLE_DATA(Particle, TimeStep);
		CALL_ASSEMBLY(Emitter->InternalPtr2);
		STORE_PARTICLE_DATA(Particle);

		--Count;
	}
}

// FIXME: this is a temporary workaround for the assembly implementation bugs
#define USE_CXX_EMISSION	1

uint32_t X86Launcher::ProcessParticles(ptcEmitter *Emitter,
	ptcParticle *StartPtr, ptcParticle *EndPtr, float TimeStep,
	ptcVector CameraCS[3], ptcVertex *Buffer, uint32_t MaxVertices) const
{
	// create local copies of the variables
	size_t NumParticles = Emitter->NumParticles;
	uint32_t VerticesLeft = MaxVertices;
	for (volatile ptcParticle *Particle = StartPtr;
		Particle < EndPtr;
		++Particle)
	{
		if (!Particle->Active)
			continue;

		LOAD_PARTICLE_DATA(Particle, TimeStep);
		CALL_ASSEMBLY(Emitter->InternalPtr3);
		STORE_PARTICLE_DATA(Particle);

		// conditional vertex emission
#if !USE_CXX_EMISSION
		// we only need to leave xmm1 and xmm2 untouched, the rest may be used
		asm(// skip vertex emission if the particle's dead
			"testl	$0xFFFFFFFF, %c[Active](%[Particle])\n"
			"jnz	.verts_left\n"
			"dec	%[NumParticles]\n"
			"jmp	.end\n"

			".verts_left:\n"
			// skip vertex emission if we've exceeded the vertex count
			"cmpl	$4, %[VerticesLeft]\n"
			"jl		.end\n"

			// load in the particle size and broadcast it to all components
			"movss	%c[Size](%[Particle]), %%xmm0\n"
			"shufps	$0x00, %%xmm0, %%xmm0\n"

			// load in 0.5 and broadcast it to all components
			"sub	$4, %%" __SP "\n"
			"movl	$0x3F000000, (%%" __SP ")\n"
			"movss	(%%" __SP "), %%xmm3\n"
			"add	$4, %%" __SP "\n"
			"shufps	$0x00, %%xmm3, %%xmm3\n"

			// load in the camera coordinate system Y and Z axes
			"mov	%[CameraCS], %%" __SI "\n"
			"movups	3 * 4(%%" __SI "), %%xmm4\n"
			"movups	6 * 4(%%" __SI "), %%xmm5\n"

			// TODO: use Rotation to modify these vectors

			// multiply those axes by particle size and 0.5
			"mulps	%%xmm0, %%xmm4\n"
			"mulps	%%xmm3, %%xmm4\n"
			"mulps	%%xmm0, %%xmm5\n"
			"mulps	%%xmm3, %%xmm5\n"

			// at this point we have:
			// xmm2 - particle location (centre)
			// xmm4 - right camera vector * particle size * 0.5
			// xmm5 - up camera vector * particle size * 0.5
			// use these to calc the different vertices

			// xmm0: vertex 1 = location + right + up
			"movaps	%%xmm2, %%xmm0\n"
			"addps	%%xmm4, %%xmm0\n"
			"addps	%%xmm5, %%xmm0\n"
			// xmm3: vertex 2 = location - right + up
			"movaps	%%xmm2, %%xmm3\n"
			"subps	%%xmm4, %%xmm3\n"
			"addps	%%xmm5, %%xmm3\n"
			// xmm6: vertex 3 = location - right - up
			"movaps	%%xmm2, %%xmm6\n"
			"subps	%%xmm4, %%xmm6\n"
			"subps	%%xmm5, %%xmm6\n"
			// xmm7: vertex 4 = location + right - up
			"movaps	%%xmm2, %%xmm7\n"
			"addps	%%xmm4, %%xmm7\n"
			"subps	%%xmm5, %%xmm7\n"

			// write vertex data to buffer
			"movups	%%xmm1, %c[Colour](%[Buffer])\n"
			"movlps	%%xmm0, %c[Location](%[Buffer])\n"
			"shufps	$0xAA, %%xmm0, %%xmm0\n"
			"movss	%%xmm0, %c[Location] + 8(%[Buffer])\n"
			"movw	$1, %c[TexCoords](%[Buffer])\n"
			"movw	$0, %c[TexCoords] + 2(%[Buffer])\n"
			"add	%[sizeof_ptcVertex], %[Buffer]\n"

			"movups	%%xmm1, %c[Colour](%[Buffer])\n"
			"movlps	%%xmm3, %c[Location](%[Buffer])\n"
			"shufps	$0xAA, %%xmm3, %%xmm3\n"
			"movss	%%xmm3, %c[Location] + 8(%[Buffer])\n"
			"movw	$0, %c[TexCoords](%[Buffer])\n"
			"movw	$0, %c[TexCoords] + 2(%[Buffer])\n"
			"add	%[sizeof_ptcVertex], %[Buffer]\n"

			"movups	%%xmm1, %c[Colour](%[Buffer])\n"
			"movlps	%%xmm6, %c[Location](%[Buffer])\n"
			"shufps	$0xAA, %%xmm6, %%xmm6\n"
			"movss	%%xmm6, %c[Location] + 8(%[Buffer])\n"
			"movw	$0, %c[TexCoords](%[Buffer])\n"
			"movw	$1, %c[TexCoords] + 2(%[Buffer])\n"
			"add	%[sizeof_ptcVertex], %[Buffer]\n"

			"movups	%%xmm1, %c[Colour](%[Buffer])\n"
			"movlps	%%xmm7, %c[Location](%[Buffer])\n"
			"shufps	$0xAA, %%xmm7, %%xmm7\n"
			"movss	%%xmm7, %c[Location] + 8(%[Buffer])\n"
			"movw	$1, %c[TexCoords](%[Buffer])\n"
			"movw	$1, %c[TexCoords] + 2(%[Buffer])\n"
			"add	%[sizeof_ptcVertex], %[Buffer]\n"

			"sub	$4, %[VerticesLeft]\n"

			".end:\n"
			: [NumParticles] "+r" (NumParticles),
				[Buffer] "+r" (Buffer),
				[VerticesLeft] "+m" (VerticesLeft)
			: [Particle] "r" (Particle),
				[CameraCS] "m" (CameraCS),
				[sizeof_ptcVertex] "i" (sizeof(ptcVertex)),
				DECLARE_STRUCT_OFFSET(ptcParticle, Active),
				DECLARE_STRUCT_OFFSET(ptcParticle, Size),
				DECLARE_STRUCT_OFFSET(ptcParticle, Rotation),
				DECLARE_STRUCT_OFFSET(ptcVertex, Colour),
				DECLARE_STRUCT_OFFSET(ptcVertex, Location),
				DECLARE_STRUCT_OFFSET(ptcVertex, TexCoords)
			: "%edx", "%" __SI, "%xmm0", "%xmm1", "%xmm2", "%xmm3",
				"%xmm4", "%xmm5", "%xmm6", "%xmm7"
		);
#else
		if (!Particle->Active)
		{
			--NumParticles;
			continue;
		}
		if (VerticesLeft < 4)
			continue;
#define CALC_VERT_LOC(RightOp, UpOp)										\
		Buffer->Location[0] = Particle->Location[0] + 0.5 * Particle->Size	\
			* (RightOp CameraCS[1][0] UpOp CameraCS[2][0]);					\
		Buffer->Location[1] = Particle->Location[1] + 0.5 * Particle->Size	\
			* (RightOp CameraCS[1][1] UpOp CameraCS[2][1]);					\
		Buffer->Location[2] = Particle->Location[2] + 0.5 * Particle->Size	\
			* (RightOp CameraCS[1][2] UpOp CameraCS[2][2])
#define COPY_COLOUR															\
		Buffer->Colour[0] = Particle->Colour[0];							\
		Buffer->Colour[1] = Particle->Colour[1];							\
		Buffer->Colour[2] = Particle->Colour[2];							\
		Buffer->Colour[3] = Particle->Colour[3]

		CALC_VERT_LOC(+, +);
		COPY_COLOUR;
		Buffer->TexCoords[0] = 1;
		Buffer->TexCoords[1] = 0;
		++Buffer;

		CALC_VERT_LOC(-, +);
		COPY_COLOUR;
		Buffer->TexCoords[0] = 0;
		Buffer->TexCoords[1] = 0;
		++Buffer;

		CALC_VERT_LOC(-, -);
		COPY_COLOUR;
		Buffer->TexCoords[0] = 0;
		Buffer->TexCoords[1] = 1;
		++Buffer;

		CALC_VERT_LOC(+, -);
		COPY_COLOUR;
		Buffer->TexCoords[0] = 1;
		Buffer->TexCoords[1] = 1;
		++Buffer;

		VerticesLeft -= 4;
#endif
	}
	Emitter->NumParticles = NumParticles;
	return (MaxVertices - VerticesLeft);
}

// get rid of unused parameter warnings
SUPPRESS_WARNING_GCC_END
