/*
particlasm threads module
Copyright (C) 2011-2012, Leszek Godlewski <lg@inequation.org>
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include "libparticlasm.h"

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

/// \sa PFNPTCCOMPILEEMITTER
uint32_t EXPORTDECL ptcCompileEmitter(ptcEmitter *emitter) {
	ptcModule	*m;
	size_t	spawnCodeBufLen = 0,
			procCodeBufLen = 0,
			dataBufLen = 0;
	void	*spawnCodeBuf, *scbcopy,
			*procCodeBuf, *pcbcopy,
			*dataBuf, *dbcopy,
			*codepage;

	// start with the simulation module
	ptcInternalMeasureModule(NULL, &spawnCodeBufLen, &procCodeBufLen,
		&dataBufLen);
	printf("libparticlasm: simulation module measurement:\n"
			"\tspawn code: %d bytes\n"
			"\tprocessing code: %d bytes\n"
			"\tdata: %d bytes\n",
			spawnCodeBufLen, procCodeBufLen, dataBufLen);
	for (m = emitter->Head; m; m = m->Header.Next) {
		ptcInternalMeasureModule(m, &spawnCodeBufLen, &procCodeBufLen,
			&dataBufLen);
		printf("libparticlasm: module %d measurement:\n"
			"\tspawn code: %d bytes\n"
			"\tprocessing code: %d bytes\n"
			"\tdata: %d bytes\n", m->Header.ModuleID,
			spawnCodeBufLen, procCodeBufLen, dataBufLen);
	}
	printf("libparticlasm: total measurements in bytes:\n"
			"\tspawn code: %d bytes\n"
			"\tprocessing code: %d bytes\n"
			"\tdata: %d bytes\n",
			spawnCodeBufLen, procCodeBufLen, dataBufLen);

	// allocate a memory page for our code
	codepage = mmap(NULL, spawnCodeBufLen + procCodeBufLen,
					PROT_READ | PROT_WRITE | PROT_EXEC,
					MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (codepage == MAP_FAILED) {
		printf("libparticlasm: code memory page allocation of %lu+%lu bytes "
			"failed: %d %s\n",
			spawnCodeBufLen, procCodeBufLen, errno, strerror(errno));
		return 0;
	}

	// the data buffer uses an ordinary malloc
	dataBuf = malloc(dataBufLen);
	// slice out the individual code buffers
	spawnCodeBuf = codepage;
	procCodeBuf = codepage + spawnCodeBufLen;

	// create copies of the pointers for the assembly to use
	scbcopy = spawnCodeBuf;
	pcbcopy = procCodeBuf;
	dbcopy = dataBuf;

	// start the compilation with the simulation preprocessing module
	ptcInternalCompileModule((void *)-1, &scbcopy, &pcbcopy, &dbcopy);
	/*printf("libparticlasm: simulation preproc. module compilation:\n"
			"\tspawn code: %d bytes\n"
			"\tprocessing code: %d bytes\n"
			"\tdata: %d bytes\n",
			(int)(scbcopy - spawnCodeBuf),
			(int)(pcbcopy - procCodeBuf),
			(int)(dbcopy - dataBuf));*/
	// compile the modules in
	for (m = emitter->Head; m; m = m->Header.Next) {
		ptcInternalCompileModule(m, &scbcopy, &pcbcopy, &dbcopy);
		/*printf("libparticlasm: module %d compilation:\n"
			"\tspawn code: %d bytes\n"
			"\tprocessing code: %d bytes\n"
			"\tdata: %d bytes\n", m->Header.ModuleID,
			(int)(scbcopy - spawnCodeBuf),
			(int)(pcbcopy - procCodeBuf),
			(int)(dbcopy - dataBuf));*/
	}
	// tip off with the the simulation postprocessing module
	ptcInternalCompileModule((void *)-2, &scbcopy, &pcbcopy, &dbcopy);
	/*printf("libparticlasm: simulation postproc. module compilation:\n"
			"\tspawn code: %d bytes\n"
			"\tprocessing code: %d bytes\n"
			"\tdata: %d bytes\n",
			(int)(scbcopy - spawnCodeBuf),
			(int)(pcbcopy - procCodeBuf),
			(int)(dbcopy - dataBuf));*/

	emitter->InternalPtr1 = dataBuf;
	emitter->InternalPtr2 = spawnCodeBuf;
	emitter->InternalPtr3 = procCodeBuf;

	return 1;
}

// entry point to the particle processing routine
// responsible for some high level organization
/// \sa PFNPTCPROCESSEMITTER
size_t EXPORTDECL ptcProcessEmitter(ptcEmitter *emitter, float step,
		ptcVector cameraCS[3], ptcVertex *buffer, size_t maxVertices) {
	size_t count;

	emitter->SpawnTimer += step;
	count = (size_t)floorf(emitter->SpawnTimer * emitter->Config.SpawnRate);
	emitter->SpawnTimer -= (float)count / emitter->Config.SpawnRate;
	count *= emitter->Config.BurstCount;
	if (count > emitter->MaxParticles - emitter->NumParticles)
		count = emitter->MaxParticles - emitter->NumParticles;
	if (count > 0) {
		//printf("libparticlasm: spawning a burst of %d particles\n", count);
		ptcInternalSpawnParticles(emitter, step, count);
	}

	// call the processing code
	count = ptcInternalProcessParticles(emitter,
		emitter->ParticleBuf, emitter->ParticleBuf + emitter->MaxParticles,
		step, cameraCS, buffer, maxVertices);

	return count;
}

/// \sa PFNPTCRELEASEEMITTER
void EXPORTDECL ptcReleaseEmitter(ptcEmitter *emitter) {
	free(emitter->InternalPtr1);
	munmap(emitter->InternalPtr2, 0);
	munmap(emitter->InternalPtr3, 0);
}
