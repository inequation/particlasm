/*
Particlasm compiled code launcher for X86 assembly
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include <cstdlib>
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
#else
	#define __AX			"eax"
	#define __BX			"ebx"
	#define __CX			"ecx"
	#define __DX			"edx"
	#define __SI			"esi"
	#define __DI			"edi"
	#define __BP			"ebp"
	#define __SP			"esp"
	#define __CALL			"call"
#endif

namespace mt19937 {
	extern void init_by_array(unsigned long init_key[], int key_length);
	extern unsigned long genrand_int32(void);
}

static void FRand(void)
{
	uint32_t RandInt = mt19937::genrand_int32();
	static float Multiplier = 1.0f / 4294967295.0f;
	asm(
		"push %1\n"
		"fld %0\n"
		"fimul (%%" __SP ")\n"
		"fld1\n"
		"fxch %%st(1)\n"
		"fsubp %%st(1)\n"
		"addl 8,%%" __SP
		: // no output registers
		: "m" (Multiplier), "r" (RandInt)
		: "%st", "%st(1)"
		);
}

static ptcExtLib ExtLib = {FRand};

X86Launcher::X86Launcher()
{
	// use the opportunity to initialize the mersenne twister
	unsigned long init[4] = {
		(unsigned long)rand(), (unsigned long)rand(),
		(unsigned long)rand(), (unsigned long)rand()
	}, length = 4;
	mt19937::init_by_array(init, length);
}

// get rid of warnings in debug builds
#ifndef NDEBUG
	SUPPRESS_WARNING_GCC_BEGIN("-Wunused-parameter")
#endif

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
	Emitter->InternalPtr1 = mmap(NULL, Size,
		PROT_READ | PROT_WRITE | PROT_EXEC,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (Emitter->InternalPtr1 == MAP_FAILED)
		return false;
	Emitter->InternalPtr2 = ((char *)Emitter->InternalPtr1) + SpawnCodeOffset;
	Emitter->InternalPtr3 = ((char *)Emitter->InternalPtr1) + ProcessCodeOffset;
	return true;
}

void X86Launcher::Unload(ptcEmitter *Emitter) const
{
	munmap(Emitter->InternalPtr1, 0);
}

size_t X86Launcher::Launch(ptcEmitter *Emitter, float Step,
	ptcVector CameraCS[3], ptcVertex *Buffer, size_t MaxVertices) const
{
	asm(
		"push %0\n"
		//__CALL " (%1)\n"
		: // no output registers
		: "r" (ExtLib.FRand)
		);
	return 0;
}

// get rid of warnings in debug builds
#ifndef NDEBUG
	SUPPRESS_WARNING_GCC_END
#endif
