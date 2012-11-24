/*
Particlasm compiled code launcher for X86 assembly
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "../Core/ParticlasmMain.h"
#include "ExternalFuncLibrary.h"
#include "X86Launcher.h"

namespace mt19937 {
	extern unsigned long genrand_int32(void);
}

static void FRand(void)
{
	uint32_t RandInt = mt19937::genrand_int32();
	static const float Multiplier = 1.0f / 4294967295.0f;
	asm(
		"push %1\n"
		"push %0\n"
		"fld (%%esp)\n"		// FIXME!!! rsp on x64
		"fimul 4(%%esp)\n"	// FIXME!!! rsp on x64
		"fld1\n"
		"fxch %%st(1)\n"
		"fsubp %%st(1)\n"
		"addl 8,%%esp"		// FIXME!!! rsp on x64
		: // no output registers
		: "r" (Multiplier), "r" (RandInt)
		: "%st", "%st(1)"
		);
}

static ptcExtLib ExtLib = {FRand};

// get rid of warnings in debug builds
#ifndef NDEBUG
	SUPPRESS_WARNING_GCC_BEGIN("-Wunused-parameter")
#endif

bool X86Launcher::LoadCachedBinaryImpl(ptcEmitter *Emitter, void *Buffer,
	const size_t Size)
{
	assert(!"Unimplemented " STRINGIFY(__FUNCTION__));
	return false;
}

size_t X86Launcher::WriteCachedBinaryImpl(ptcEmitter *Emitter,
	PFNWRITE WriteCallback)
{
	assert(!"Unimplemented " STRINGIFY(__FUNCTION__));
	return 0;
}

void X86Launcher::LoadRawBinaryImpl(ptcEmitter *Emitter, void *Buffer,
	const size_t Size, const size_t DataOffset, const size_t SpawnCodeOffset,
	const size_t ProcessCodeOffset)
{
	assert(!"Unimplemented " STRINGIFY(__FUNCTION__));
}

size_t X86Launcher::LaunchImpl(ptcEmitter *Emitter, float Step,
	ptcVector CameraCS[3], ptcVertex *Buffer, size_t MaxVertices)
{
	asm(
		"push %0\n"
		"addl 8,%%esp\n"	// FIXME!!! rsp on x64
		: // no output registers
		: "r" (ExtLib.FRand)
		);
	return 0;
}

// get rid of warnings in debug builds
#ifndef NDEBUG
	SUPPRESS_WARNING_GCC_END
#endif
