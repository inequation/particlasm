/*
particlasm headless benchmark
Copyright (C) 2012, Leszek Godlewski <lg@inequation.org>
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <time.h>
#include <SDL/SDL.h>

// particlasm functions
#include "../libparticlasm/libparticlasm.h"
PFNPTCCOMPILEEMITTER	ptcCompileEmitter;
PFNPTCPROCESSEMITTER	ptcProcessEmitter;
PFNPTCRELEASEEMITTER	ptcReleaseEmitter;

// test parameters
#define USE_CPP_REFERENCE_IMPLEMENTATION
#define TEST_TIME		30
#define TEST_FRAMERATE	60

#ifdef USE_CPP_REFERENCE_IMPLEMENTATION
extern PTC_ATTRIBS unsigned int ref_ptcCompileEmitter(ptcEmitter *emitter);
extern PTC_ATTRIBS uint32_t ref_ptcProcessEmitter(ptcEmitter *emitter,
	float step, ptcVector cameraCS[3], ptcVertex *buffer, uint32_t maxVertices);
extern PTC_ATTRIBS void ref_ptcReleaseEmitter(ptcEmitter *emitter);
#else
void *libparticlasmHandle = NULL;
#endif // USE_CPP_REFERENCE_IMPLEMENTATION

extern size_t Fire(ptcEmitter **emitters);

size_t MAX_PARTICLES;

size_t ptc_nemitters;
ptcEmitter *ptc_emitters;
volatile size_t ptc_nvertices;
ptcParticle *ptc_particles;
ptcVertex *ptc_vertices;

size_t test_start_msec;

bool InitParticlasm() {
#ifdef USE_CPP_REFERENCE_IMPLEMENTATION
	ptcCompileEmitter = ref_ptcCompileEmitter;
	ptcProcessEmitter = ref_ptcProcessEmitter;
	ptcReleaseEmitter = ref_ptcReleaseEmitter;
	return true;
#else
	libparticlasmHandle = dlopen("/home/inequation/projects/particlasm/bin/Debug/libparticlasm.so", RTLD_NOW);
	if (!libparticlasmHandle) {
		printf("dlerror: %s\n", dlerror());
		return false;
	}
	ptcCompileEmitter = (PFNPTCCOMPILEEMITTER)dlsym(libparticlasmHandle, "ptcCompileEmitter");
	ptcProcessEmitter = (PFNPTCPROCESSEMITTER)dlsym(libparticlasmHandle, "ptcProcessEmitter");
	ptcReleaseEmitter = (PFNPTCRELEASEEMITTER)dlsym(libparticlasmHandle, "ptcReleaseEmitter");
	if (ptcCompileEmitter && ptcProcessEmitter && ptcReleaseEmitter)
		return true;
	printf("dlerror: %s\n", dlerror());
	dlclose(libparticlasmHandle);
	return false;
#endif // USE_CPP_REFERENCE_IMPLEMENTATION
}

void FreeParticlasm() {
#ifdef USE_CPP_REFERENCE_IMPLEMENTATION
#else
	if (libparticlasmHandle)
		dlclose(libparticlasmHandle);
#endif // USE_CPP_REFERENCE_IMPLEMENTATION
}

int main(int argc, char *argv[]) {
	size_t i, j;
	static ptcVector cameraCS[3] = {
		{1, 0, 0}, {0, 1, 0}, {0, 0, 1}
	};

	if (argc < 2) {
		fprintf(stderr, "Please provide max particle count as argument.\n");
		return 1;
	}
	MAX_PARTICLES = atoi(argv[1]);
	ptc_particles = malloc(sizeof(*ptc_particles) * MAX_PARTICLES);
	ptc_vertices = malloc(sizeof(*ptc_vertices) * MAX_PARTICLES * 4);

	if (SDL_Init(SDL_INIT_TIMER) < 0) {
	    fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
	    return 1;
	}

	if (!InitParticlasm()) {
		fprintf(stderr, "Could not initialize particlasm.\n");
		return 1;
	}

	// initialize random number generator
	srand((unsigned int)time(NULL));

	printf("particlasm headless benchmark\n"
		"Test parameters:\n"
		"    Implementation: "
#ifdef USE_CPP_REFERENCE_IMPLEMENTATION
			"C++\n"
#else
			"assembly\n"
#endif
		"    Sim. time:      %3.2f\n"
		"    Sim. framerate: %3.2f\n"
		"    Max particles:  %d\n"
		"    Max vertices:   %d\n\n",
		(float)TEST_TIME, (float)TEST_FRAMERATE, MAX_PARTICLES,
			MAX_PARTICLES * 4);

	ptc_nemitters = Fire(&ptc_emitters);
	for (i = 0; i < ptc_nemitters; ++i) {
		ptc_emitters[i].ParticleBuf = ptc_particles;
		ptc_emitters[i].MaxParticles = MAX_PARTICLES;
		if (!ptcCompileEmitter(&ptc_emitters[i])) {
			fprintf(stderr, "Could not compile emitter.\n");
			return 2;
		}
	}

	printf("Starting test... ");
	fflush(stdout);
	test_start_msec = SDL_GetTicks();
	for (i = 0; i < (TEST_TIME * TEST_FRAMERATE); ++i) {
		ptc_nvertices = 0;
		for (j = 0; j < ptc_nemitters; ++j) {
			ptc_nvertices += ptcProcessEmitter(&ptc_emitters[j],
				1.f / (float)TEST_FRAMERATE, cameraCS,
				ptc_vertices + ptc_nvertices,
				(MAX_PARTICLES * 4) - ptc_nvertices);
		}
	}
	test_start_msec = SDL_GetTicks() - test_start_msec;
	printf("Done simulating %d frames.\n%d ms, avg. %3.2f ms/frame\n",
		i, test_start_msec, (float)test_start_msec
			/ (float)(TEST_TIME * TEST_FRAMERATE));

	for (i = 0; i < ptc_nemitters; ++i)
		ptcReleaseEmitter(&ptc_emitters[i]);
	// clean up particlasm
	FreeParticlasm();

	free(ptc_vertices);
	free(ptc_particles);

	return 0;
}
