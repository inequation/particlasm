/*
particlasm threads module
Copyright (C) 2011, Leszek Godlewski <lg@inequation.org>
*/

#include <math.h>
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

// declaration of the assembly particle spawning function
extern void ptcInternalSpawnParticles(ptcEmitter *emitter, float step,
	uint32_t count);
// declaration of the assembly particle advancing function
extern void ptcInternalProcessParticles(ptcEmitter *emitter, float step,
	uint32_t count);

// entry point to the particle processing routine
// responsible for some high level organization
uint32_t EXPORTDECL ptcProcessEmitter(ptcEmitter *emitter, float step,
		ptcVector cameraCS[3], ptcVertex *buffer, uint32_t maxVertices) {
	float bursts;
	int count;

	emitter->SpawnTimer += step;
	bursts = floorf(emitter->SpawnTimer * emitter->SpawnRate);
	emitter->SpawnTimer -= bursts;

	count = ((int)bursts * emitter->BurstCount)
		% (emitter->MaxParticles - emitter->NumParticles);
	ptcInternalSpawnParticles(emitter, step, count);

	return 0;
}
