/*
particlasm headless benchmark
Copyright (C) 2012, Leszek Godlewski <lg@inequation.org>
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <time.h>
#include <sys/time.h>

// particlasm functions
#include "../libparticlasm/libparticlasm.h"
PFNPTCCOMPILEEMITTER	ptcCompileEmitter;
PFNPTCPROCESSEMITTER	ptcProcessEmitter;
PFNPTCRELEASEEMITTER	ptcReleaseEmitter;

// test parameters
#define TEST_TIME		30
#define TEST_FRAMERATE	60

extern PTC_ATTRIBS unsigned int ref_ptcCompileEmitter(ptcEmitter *emitter);
extern PTC_ATTRIBS uint32_t ref_ptcProcessEmitter(ptcEmitter *emitter,
	float step, ptcVector cameraCS[3], ptcVertex *buffer, uint32_t maxVertices);
extern PTC_ATTRIBS void ref_ptcReleaseEmitter(ptcEmitter *emitter);

void *libparticlasmHandle = NULL;

extern size_t Fire(ptcEmitter **emitters);

size_t MAX_PARTICLES;

size_t ptc_nemitters;
ptcEmitter *ptc_emitters;
volatile size_t ptc_nvertices;
ptcParticle *ptc_particles;
ptcVertex *ptc_vertices;

size_t test_start_msec;

bool InitParticlasm(bool cpp) {
	if (cpp) {
		ptcCompileEmitter = ref_ptcCompileEmitter;
		ptcProcessEmitter = ref_ptcProcessEmitter;
		ptcReleaseEmitter = ref_ptcReleaseEmitter;
		return true;
	} else {
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
	}
}

void FreeParticlasm() {
	if (libparticlasmHandle)
		dlclose(libparticlasmHandle);
	ptcCompileEmitter = NULL;
	ptcProcessEmitter = NULL;
	ptcReleaseEmitter = NULL;
}

struct timeval start;

void InitTicks() {
	gettimeofday(&start, NULL);
}

unsigned int GetTicks() {
	unsigned int ticks;
	struct timeval now;

	gettimeofday(&now, NULL);
	ticks = (now.tv_sec - start.tv_sec) * 1000 + (now.tv_usec -
		start.tv_usec) / 1000;
    return (ticks);
}

bool Benchmark(bool cpp) {
	size_t i, j;
	static ptcVector cameraCS[3] = {
		{1, 0, 0}, {0, 1, 0}, {0, 0, 1}
	};

	if (!InitParticlasm(cpp)) {
		fprintf(stderr, "Could not initialize %s implementation.\n",
			cpp ? "C++" : "assembly");
		return false;
	}

	memset(ptc_particles, 0, sizeof(*ptc_particles) * MAX_PARTICLES);
	memset(ptc_vertices, 0, sizeof(*ptc_vertices) * MAX_PARTICLES * 4);

	printf("\nTest parameters:\n"
		"    Implementation: %s\n"
		"    Sim. time:      %3.2f\n"
		"    Sim. framerate: %3.2f\n"
		"    Max particles:  %d\n"
		"    Max vertices:   %d\n\n",
		cpp ? "C++" : "assembly",
		(float)TEST_TIME, (float)TEST_FRAMERATE, MAX_PARTICLES,
			MAX_PARTICLES * 4);

	ptc_nemitters = Fire(&ptc_emitters);
	for (i = 0; i < ptc_nemitters; ++i) {
		ptc_emitters[i].ParticleBuf = ptc_particles;
		ptc_emitters[i].MaxParticles = MAX_PARTICLES;
		if (!ptcCompileEmitter(&ptc_emitters[i])) {
			fprintf(stderr, "Could not compile emitter.\n");
			return false;
		}
	}

	printf("Starting test... ");
	fflush(stdout);
	test_start_msec = GetTicks();
	for (i = 0; i < (TEST_TIME * TEST_FRAMERATE); ++i) {
		ptc_nvertices = 0;
		for (j = 0; j < ptc_nemitters; ++j) {
			ptc_nvertices += ptcProcessEmitter(&ptc_emitters[j],
				1.f / (float)TEST_FRAMERATE, cameraCS,
				ptc_vertices + ptc_nvertices,
				(MAX_PARTICLES * 4) - ptc_nvertices);
		}
	}
	test_start_msec = GetTicks() - test_start_msec;
	printf("Done simulating %d frames.\n%d ms, avg. %3.2f ms/frame\n",
		i, test_start_msec, (float)test_start_msec
			/ (float)(TEST_TIME * TEST_FRAMERATE));

	for (i = 0; i < ptc_nemitters; ++i)
		ptcReleaseEmitter(&ptc_emitters[i]);
	// clean up particlasm
	FreeParticlasm();
	return true;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Please provide max particle count as argument.\n");
		return 1;
	}
	MAX_PARTICLES = atoi(argv[1]);
	ptc_particles = malloc(sizeof(*ptc_particles) * MAX_PARTICLES);
	ptc_vertices = malloc(sizeof(*ptc_vertices) * MAX_PARTICLES * 4);

	InitTicks();

	// initialize random number generator
	srand((unsigned int)time(NULL));

	printf("particlasm headless benchmark\n");
	Benchmark(true);
	Benchmark(false);

	free(ptc_vertices);
	free(ptc_particles);

	return 0;
}
