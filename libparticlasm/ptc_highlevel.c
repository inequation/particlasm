/*
particlasm threads module
Copyright (C) 2011, Leszek Godlewski <lg@inequation.org>
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
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

// declaration of the assembly module size measuring function
extern void ptcInternalMeasureModule(ptcModule *module,
	uint32_t *spawnCodeBufLenPtr, uint32_t *processCodeBufLenPtr,
	uint32_t *dataBufLenPtr);
// declaration of the assembly module compilation function
extern void ptcInternalCompileModule(ptcModule *module,
	void **spawnCodeBufPtr, void **processCodeBufPtr,
	void **dataBufPtr);
// declaration of the assembly particle spawning function
extern void ptcInternalSpawnParticles(ptcEmitter *emitter, float step,
	uint32_t count);
// declaration of the assembly particle advancing function
extern uint32_t ptcInternalProcessParticles(ptcEmitter *emitter,
	ptcParticle *startPtr, ptcParticle *endPtr, float step,
	ptcVector cameraCS[3], ptcVertex *buffer, uint32_t maxVertices);


uint32_t EXPORTDECL ptcCompileEmitter(ptcEmitter *emitter) {
	ptcModule	*m;
	uint32_t	spawnCodeBufLen = 0,
				procCodeBufLen = 0,
				dataBufLen = 0;
	void		*spawnCodeBuf, *scbcopy,
				*procCodeBuf, *pcbcopy,
				*dataBuf, *dbcopy,
				*codepage;

	// start with the simulation module
	ptcInternalMeasureModule(NULL, &spawnCodeBufLen, &procCodeBufLen,
		&dataBufLen);
	/*printf("libparticlasm: simulation module measurement:\n"
			"\tspawn code: %d bytes\n"
			"\tprocessing code: %d bytes\n"
			"\tdata: %d bytes\n",
			spawnCodeBufLen, procCodeBufLen, dataBufLen);*/
	for (m = emitter->Head; m; m = m->Header.Next) {
		ptcInternalMeasureModule(m, &spawnCodeBufLen, &procCodeBufLen,
			&dataBufLen);
		/*printf("libparticlasm: module %d measurement:\n"
			"\tspawn code: %d bytes\n"
			"\tprocessing code: %d bytes\n"
			"\tdata: %d bytes\n", m->Header.ModuleID,
			spawnCodeBufLen, procCodeBufLen, dataBufLen);*/
	}
	/*printf("libparticlasm: total measurements in bytes:\n"
			"\tspawn code: %d bytes\n"
			"\tprocessing code: %d bytes\n"
			"\tdata: %d bytes\n",
			spawnCodeBufLen, procCodeBufLen, dataBufLen);*/

	// allocate a memory page for our code
	codepage = mmap(NULL, spawnCodeBufLen + procCodeBufLen,
					PROT_READ | PROT_WRITE | PROT_EXEC,
					MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (codepage == MAP_FAILED) {
		printf("libparticlasm: code memory page allocation failed: %d\n", errno);
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
uint32_t EXPORTDECL ptcProcessEmitter(ptcEmitter *emitter, float step,
		ptcVector cameraCS[3], ptcVertex *buffer, uint32_t maxVertices) {
	float bursts;
	uint32_t count;

	emitter->SpawnTimer += step;
	if (emitter->SpawnTimer > 0.f) {
		bursts = floorf(emitter->SpawnTimer * emitter->SpawnRate);
		emitter->SpawnTimer -= bursts;
	} else
		bursts = 0.f;

	count = (int)bursts * emitter->BurstCount;
	if (count > emitter->MaxParticles - emitter->NumParticles)
		count = emitter->MaxParticles - emitter->NumParticles;
	if (count > 0) {
		printf("libparticlasm: spawning a burst of %d particles\n", count);
		ptcInternalSpawnParticles(emitter, step, count);
	}

	// call the processing code
	count = ptcInternalProcessParticles(emitter,
		emitter->ParticleBuf, emitter->ParticleBuf + emitter->MaxParticles,
		step, cameraCS, buffer, maxVertices);

	return count;
}

uint32_t EXPORTDECL ptcReleaseEmitter(ptcEmitter *emitter) {
	free(emitter->InternalPtr1);
	munmap(emitter->InternalPtr2, 0);
	munmap(emitter->InternalPtr3, 0);
	return 1;
}
