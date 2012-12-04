/*
particlasm headless benchmark
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#if defined(WIN32) || defined(_WIN32) || defined(_WIN32_WINNT)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <time.h>
	#define PATH_SEPARATOR	"\\"
	#define LOCAL_PATH
	#define SO_EXT			".dll"
	typedef HMODULE			SO_HANDLE;
	#define dlopen(a, b)	LoadLibrary(a)
	#define dlsym(a, b)		GetProcAddress(a, b)
	#define dlclose(a)		FreeLibrary(a)
#else
	#include <unistd.h>
	#include <dlfcn.h>
	#include <sys/time.h>
	#define PATH_SEPARATOR	"/"
	#define LOCAL_PATH		"./"
	#define SO_EXT			".so"
	typedef void			*SO_HANDLE;
	// change this to 0 if your compiler complains about dladdr
	#define HAVE_DLADDR		1
#endif // WIN32

// particlasm functions
#include <libparticlasm2.h>
PFNPTCGETAPI	ptcGetAPI;
ptcAPIExports	ptcAPI;

// test parameters
#define TEST_TIME		30
#define TEST_FRAMERATE	60

// declare the right target platform
#if defined(_M_AMD64) || defined(amd64) || defined (__amd64__)
	#define LOCAL_TARGET	ptcTarget_x86_64
#else
	#define LOCAL_TARGET	ptcTarget_x86
#endif

extern PTC_ATTRIBS uint32_t ref_ptcCompileEmitter(ptcEmitter *emitter);
extern PTC_ATTRIBS uint32_t ref_ptcProcessEmitter(ptcEmitter *emitter,
	float step, ptcVector cameraCS[3], ptcVertex *buffer, uint32_t maxVertices);
extern PTC_ATTRIBS void ref_ptcReleaseEmitter(ptcEmitter *emitter);

extern "C" size_t Fire(ptcEmitter **emitters);

SO_HANDLE libparticlasmHandle = NULL;

size_t MAX_PARTICLES;

size_t ptc_nemitters;
ptcEmitter *ptc_emitters;
volatile size_t ptc_nvertices;
ptcParticle *ptc_particles;
ptcVertex *ptc_vertices;

size_t test_start_msec;

size_t cpp_msec, asm_msec;

const char *GetPathToSelf() {
#if defined(WIN32) || defined(_WIN32) || defined(_WIN32_WINNT)
	static char path[MAX_PATH];
	GetModuleFileName(NULL, path, sizeof(path));
	return path;
#elif HAVE_DLADDR
	// this code is not exactly portable between different C libraries since it
	// relies on an extension first introduced in SunOS/Solaris and then
	// mirrored in Glibc/FreeBSD; if compilation fails on your system, try
	// commenting out HAVE_DLADDR at the beginning of the file
	static Dl_info info;
	if (!dladdr((void *)GetPathToSelf, &info))
		return NULL;
	return info.dli_fname;
#else
	// this is more portable, but less reliable - if reading /proc/self/exe
	// fails, current working directory is returned instead, which is not always
	// what we need
	static char path[256];
	if (readlink("/proc/self/exe", path, sizeof(path) - 1) > 0) {
		path[sizeof(path) - 1] = 0;
		return path;
	}
	if (getcwd(path, sizeof(path) - 1) == NULL)
		return NULL;
	path[sizeof(path) - 1] = 0;
	// tack on a trailing slash so the path may be recovered
	strcat(path, "/");
	return path;
#endif
}

bool InitParticlasm(bool cpp) {
	if (cpp) {
		ptcAPI.CompileEmitter = ref_ptcCompileEmitter;
		ptcAPI.ProcessEmitter = ref_ptcProcessEmitter;
		ptcAPI.ReleaseEmitter = ref_ptcReleaseEmitter;
		return true;
	} else {
		libparticlasmHandle = dlopen(LOCAL_PATH "libparticlasm2-" PLATFORM "-" ARCH SO_EXT, RTLD_NOW);
		if (!libparticlasmHandle) {
#if defined(WIN32) || defined(_WIN32) || defined(_WIN32_WINNT)
			TCHAR *errStr;
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
				| FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(),
				LANG_USER_DEFAULT, (LPSTR)&errStr, 0, NULL);
			printf("GetLastError: %s\n", errStr);
			LocalFree((HLOCAL)errStr);
#else
			printf("dlerror: %s\n", dlerror());
#endif
			return false;
		}
		ptcGetAPI = (PFNPTCGETAPI)dlsym(libparticlasmHandle, PTC_ENTRY_POINT);
		if (ptcGetAPI && ptcGetAPI(PTC_API_VERSION, &ptcAPI)
			&& ptcAPI.InitializeTarget(LOCAL_TARGET, NULL))
			return true;
#if defined(WIN32) || defined(_WIN32) || defined(_WIN32_WINNT)
		TCHAR *errStr;
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
			| FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(),
			LANG_USER_DEFAULT, (LPSTR)&errStr, 0, NULL);
		printf("GetLastError: %s\n", errStr);
		LocalFree((HLOCAL)errStr);
#else
		printf("dlerror: %s\n", dlerror());
#endif
		dlclose(libparticlasmHandle);
		return false;
	}
}

void FreeParticlasm() {
	if (libparticlasmHandle) {
		ptcAPI.ShutdownTarget(NULL);
		dlclose(libparticlasmHandle);
	}
}

#if defined(WIN32) || defined(_WIN32) || defined(_WIN32_WINNT)
static LARGE_INTEGER start;

void InitTicks() {
	QueryPerformanceCounter(&start);
}

unsigned int GetTicks() {
	LARGE_INTEGER ticks;
	LARGE_INTEGER now;
	LARGE_INTEGER frequency;

	QueryPerformanceCounter(&now);
	QueryPerformanceFrequency(&frequency);
	frequency.QuadPart /= 1000;	// seconds to milliseconds
	ticks.QuadPart = (now.QuadPart - start.QuadPart) / frequency.QuadPart;
    return (unsigned int)ticks.QuadPart;
}
#else
static struct timeval start;

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
#endif

bool Benchmark(bool cpp) {
	size_t i, j;
	static ptcVector cameraCS[3] = {
		{1, 0, 0}, {0, 1, 0}, {0, 0, 1}
	};

	printf("\nInitializing %s implementation: ", cpp ? "C++" : "assembly");
	fflush(stdout);
	if (!InitParticlasm(cpp)) {
		fprintf(stderr, "Could not initialize implementation.\n");
		return false;
	}
	printf("Done.\n");

	memset(ptc_particles, 0, sizeof(*ptc_particles) * MAX_PARTICLES);
	memset(ptc_vertices, 0, sizeof(*ptc_vertices) * MAX_PARTICLES * 4);

	printf("Test parameters:\n"
		"    Implementation: %s\n"
		"    Sim. time:      %3.2f\n"
		"    Sim. framerate: %3.2f\n"
		"    Max particles:  %d\n"
		"    Max vertices:   %d\n\n",
		cpp ? "C++" : "assembly",
		(float)TEST_TIME, (float)TEST_FRAMERATE, (int)MAX_PARTICLES,
			(int)MAX_PARTICLES * 4);

	ptc_nemitters = Fire(&ptc_emitters);
	printf("Compiling emitter... ");
	fflush(stdout);
	for (i = 0; i < ptc_nemitters; ++i) {
		ptc_emitters[i].ParticleBuf = ptc_particles;
		ptc_emitters[i].MaxParticles = MAX_PARTICLES;
		if (!ptcAPI.CompileEmitter(&ptc_emitters[i])) {
			fprintf(stderr, "Could not compile emitter.\n");
			return false;
		}
	}

	printf("Done.\n"
		"Starting test... ");
	fflush(stdout);
	test_start_msec = GetTicks();
	for (i = 0; i < (TEST_TIME * TEST_FRAMERATE); ++i) {
		ptc_nvertices = 0;
		for (j = 0; j < ptc_nemitters; ++j) {
			ptc_nvertices += ptcAPI.ProcessEmitter(&ptc_emitters[j],
				1.f / (float)TEST_FRAMERATE, cameraCS,
				ptc_vertices + ptc_nvertices,
				(MAX_PARTICLES * 4) - ptc_nvertices);
		}
	}
	test_start_msec = GetTicks() - test_start_msec;
	printf("Done simulating %d frames.\n%d ms, avg. %3.2f ms/frame\n",
		(int)i, (int)test_start_msec, (float)test_start_msec
			/ (float)(TEST_TIME * TEST_FRAMERATE));
	if (cpp)
		cpp_msec = test_start_msec;
	else
		asm_msec = test_start_msec;

	for (i = 0; i < ptc_nemitters; ++i)
		ptcAPI.ReleaseEmitter(&ptc_emitters[i]);
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
	ptc_particles = (ptcParticle *)malloc(sizeof(*ptc_particles) * MAX_PARTICLES);
	ptc_vertices = (ptcVertex *)malloc(sizeof(*ptc_vertices) * MAX_PARTICLES * 4);

	InitTicks();

	// initialize random number generator
	srand((unsigned int)time(NULL));

	printf("particlasm headless benchmark\n");
	Benchmark(true);
	Benchmark(false);
	printf("\nAssembly speed-up ratio: %3.4f\n", (float)cpp_msec / (float)asm_msec);

	free(ptc_vertices);
	free(ptc_particles);

	return 0;
}
