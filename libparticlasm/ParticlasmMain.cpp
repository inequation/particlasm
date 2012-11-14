/*
Particlasm entry point module
Copyright (C) 2011-2012, Leszek Godlewski <github@inequation.org>
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <cstdarg>
#include <sys/mman.h>
#include "libparticlasm.h"
#include "CodeGeneratorInterface.h"
#include "X86Assembly/X86AssemblyGenerator.h"

// choose an appropriate symbol export declaration
#if (defined(WIN32) || defined(__WIN32__))
	#define EXPORTDECL	__declspec(dllexport)
#elif defined(__BEOS__) && !defined(__GNUC__)
	#define EXPORTDECL	__declspec(export)
#elif defined(__GNUC__) && __GNUC__ >= 4
	#define EXPORTDECL	__attribute__ ((visibility("default")))
#else
	#define EXPORTDECL
#endif

/// Internal assembly module size measuring procedure. Increases the counters
/// pointed at with the sizes of the corresponding buffers.
///	\param	module					module to measure
/// \param	spawnCodeBufLenPtr		pointer to spawn code buffer length
/// \param	processCodeBufLenPtr	pointer to processing code buffer length
/// \param	dataBufLenPtr			pointer to spawn code buffer length
extern void ptcInternalMeasureModule(ptcModule *module,
	size_t *spawnCodeBufLenPtr, size_t *processCodeBufLenPtr,
	size_t *dataBufLenPtr);

/// Internal assembly module compilation procedure. Puts the resulting code and
/// data in the indicated buffers and advances the indicated pointers.
/// \param	module					module whose code will be compiled
/// \param	spawnCodeBufPtr			pointer to a pointer to the spawn code buffer, will be advanced by the compiler
/// \param	processCodeBufPtr		pointer to a pointer to the processing code buffer, will be advanced by the compiler
/// \param	dataBufPtr				pointer to a pointer to the data buffer, will be advanced by the compiler
extern void ptcInternalCompileModule(ptcModule *module,
	void **spawnCodeBufPtr, void **processCodeBufPtr,
	void **dataBufPtr);

/// Internal particle spawning procedure. Performs the control logic and calls
/// the spawn code buffer accordingly.
/// \param	emitter					emitter to process
/// \param	step					simulation step time
/// \param	count					number of particles to spawn
extern void ptcInternalSpawnParticles(ptcEmitter *emitter, float step,
	size_t count);

/// Internal particle processing procedure. Performs the control logic and calls
/// the processing code buffer accordingly, emitting vertices to the given
/// buffer.
/// \param	emitter					emitter to process
/// \param	startPtr				pointer to the start of the particle buffer segment to process
/// \param	endPtr					pointer to just beyond the end of the particle buffer segment to process
/// \param	step					simulation step time
/// \param	cameraCS				camera coordinate system - 3 unit-length vectors: forward, right and up
/// \param	buffer					buffer to emit vertices to
/// \param	maxVertices				maximum number of vertices to emit
extern size_t ptcInternalProcessParticles(ptcEmitter *emitter,
	ptcParticle *startPtr, ptcParticle *endPtr, float step,
	ptcVector cameraCS[3], ptcVertex *buffer, size_t maxVertices);

FILE *Code;

static void CodePrintf(const char *fmt, ...)
{
	va_list argptr;

	va_start(argptr, fmt);
	vfprintf(Code, fmt, argptr);
	va_end(argptr);
}

/// \sa PFNPTCCOMPILEEMITTER
extern "C" uint32_t EXPORTDECL ptcCompileEmitter(ptcEmitter *emitter)
{

	Code = fopen("code.asm", "w");
	CodeGenerationContext Context(emitter, CodePrintf);
	X86AssemblyGenerator Generator(X86AssemblyGenerator::ARCH_x86,
		X86AssemblyGenerator::PLATFORM_Linux);
	Generator.Generate(Context);
	printf("Generator returned %s in module #%d in stage %s\n",
		Context.GetResultString(), Context.CurrentModuleIndex,
		Context.GetStageString());
	fclose(Code);
	return 0;
}

// entry point to the particle processing routine
// responsible for some high level organization
/// \sa PFNPTCPROCESSEMITTER
extern "C" size_t EXPORTDECL ptcProcessEmitter(ptcEmitter *emitter, float step,
		ptcVector cameraCS[3], ptcVertex *buffer, size_t maxVertices)
{
	return 0;
}

/// \sa PFNPTCRELEASEEMITTER
extern "C" void EXPORTDECL ptcReleaseEmitter(ptcEmitter *emitter)
{
}
