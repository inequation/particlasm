/*
Particlasm entry point module
Copyright (C) 2011-2012, Leszek Godlewski <github@inequation.org>
*/

#include <cstdlib>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <cstdarg>
#include <ctime>

#ifdef NDEBUG
	#define Debugf
#else
	#include <cstdio>
	#define Debugf	printf
#endif // NDEBUG

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

#include "ParticlasmMain.h"
#include "CodeGeneratorInterface.h"
#include "LauncherInterface.h"

#if WITH_RUNTIMEINTERPRETER
	#include "../RuntimeInterpreter/RuntimeInterpreter.h"
#endif // WITH_RUNTIMEINTERPRETER

#if WITH_X86ASSEMBLY
	#include "../X86Assembly/X86Generator.h"
	#include "../X86Assembly/X86Launcher.h"
#endif // WITH_X86ASSEMBLY

FILE *GSourceCode;
static const CodeGeneratorInterface	*GGenerator	= NULL;
static const LauncherInterface		*GLauncher	= NULL;

static void OpenSourceFile(const char *Path)
{
	GSourceCode = fopen(Path, "w");
}

static void CodeEmitf(const char *Fmt, ...)
{
	va_list argptr;

	va_start(argptr, Fmt);
	vfprintf(GSourceCode, Fmt, argptr);
	va_end(argptr);
}

static void CloseSourceFile()
{
	fclose(GSourceCode);
}

typedef std::map<void *, std::pair<int, size_t> > FileMap;
static FileMap IntermediateFiles;

static const void *OpenIntermediateFile(const char *Path, FileAccessMode Mode,
	size_t *OutFileSize)
{
	int FileFlags;
	switch (Mode & 0x03)
	{
		case FAM_Read:		FileFlags = O_RDONLY;						break;
		case FAM_Write:		FileFlags = O_WRONLY | O_CREAT | O_TRUNC;	break;
		case FAM_ReadWrite:	FileFlags = O_RDWR;							break;
		default:			return NULL;
	}

	int fd = open(Path, FileFlags);
	if (fd == -1)
		return NULL;

	struct stat FileStat;
	if (fstat(fd, &FileStat))
	{
		close(fd);
		return NULL;
	}
	if (OutFileSize)
		*OutFileSize = FileStat.st_size;

	size_t MappingLength;
	switch(Mode & 0x03)
	{
		case FAM_Read:		MappingLength = FileStat.st_size;			break;
		case FAM_Write:		MappingLength = (Mode & ~0x03) >> 2;		break;
		case FAM_ReadWrite:	MappingLength = 2 * FileStat.st_size;		break;
		case FAM_PADDING:	break;	// shut up compiler
	}
	void *Ptr = mmap(NULL, MappingLength,
		PROT_READ, MAP_SHARED | MAP_FILE, fd, 0);
	if (Ptr == MAP_FAILED)
	{
		close(fd);
		return NULL;
	}

	IntermediateFiles.insert(FileMap::value_type
		(Ptr, std::pair<int, size_t>(fd, FileStat.st_size)));

	return Ptr;
}

static void CloseIntermediateFile(const void *FilePtr)
{
	FileMap::iterator It = IntermediateFiles.find((void *)FilePtr);
	if (It == IntermediateFiles.end())
		return;
	munmap(It->first, It->second.second);
	close(It->second.first);
	IntermediateFiles.erase(It);
}

// get rid of warnings in debug builds
#ifndef NDEBUG
	SUPPRESS_WARNING_GCC_BEGIN("-Wunused-parameter")
#endif

static void DeleteIntermediateFile(const char *Path)
{
#ifdef NDEBUG
	remove(Path);
#endif
}

PTC_ATTRIBS uint32_t ptcQueryTargetSupport(ptcTarget target)
{
	switch (target)
	{
#if WITH_RUNTIMEINTERPRETER
		case ptcTarget_RuntimeInterpreter:
			return 1;
#endif // WITH_RUNTIMEINTERPRETER
#if WITH_X86ASSEMBLY
	#if defined(_M_AMD64) || defined(amd64) || defined (__amd64__)
		case ptcTarget_x86_64:
	#else
		case ptcTarget_x86:
	#endif // amd64
			return 1;
#endif // WITH_X86ASSEMBLY
		default:
			return 0;
	}
}

PTC_ATTRIBS uint32_t ptcInitializeTarget(ptcTarget target, void *privateData)
{
	// don't succeed if there is already an initialized target
	if (GLauncher || GGenerator)
		return 0;

	switch (target)
	{
#if WITH_RUNTIMEINTERPRETER
		case ptcTarget_RuntimeInterpreter:
			{
				const RuntimeInterpreter *RI = new RuntimeInterpreter();
				GGenerator = RI;
				GLauncher = RI;
			}
			return 1;
#endif // WITH_RUNTIMEINTERPRETER
#if WITH_X86ASSEMBLY
	#if defined(_M_AMD64) || defined(amd64) || defined (__amd64__)
		case ptcTarget_x86_64:
	#else
		case ptcTarget_x86:
	#endif // amd64
			GGenerator = new X86Generator(
	#if defined(_M_AMD64) || defined(amd64) || defined (__amd64__)
				X86Generator::ARCH_x86_64
	#else
				X86Generator::ARCH_x86
	#endif
			);
			GLauncher = new X86Launcher();
			return 1;
#endif // WITH_X86ASSEMBLY
		default:
			return 0;
	}
	return 0;
}

PTC_ATTRIBS void ptcShutdownTarget(void *privateData)
{
	if (GGenerator)
	{
		delete GGenerator;
		GGenerator = NULL;
	}
	if (GLauncher)
	{
		delete GLauncher;
		GLauncher = NULL;
	}
}

PTC_ATTRIBS uint32_t ptcCompileEmitter(ptcEmitter *emitter)
{
	static char SourceBaseName[256] = {0};

	snprintf(SourceBaseName, sizeof(SourceBaseName) - 1,
		"%sparticlasm_%d_%d",
#if defined(WIN32) || defined(__WIN32__)
		"%TEMP%\\", (int)GetCurrentProcessId(),
#else
		"/tmp/", getpid(),
#endif // WIN32
		(int)time(NULL) ^ rand());

	CodeGenerationContext GenContext(emitter, SourceBaseName, OpenSourceFile,
		CodeEmitf, CloseSourceFile);

	GGenerator->Generate(GenContext);
	Debugf("Generation finished with %s (arg %d) in module #%d in stage %s\n",
		GenContext.GetResultString(), GenContext.ResultArgument,
		GenContext.CurrentModuleIndex, GenContext.GetStageString());

	const uint32_t Result = GenContext.Result == GR_Success;
	if (Result)
	{
		static char OutputBuffer[16384], BinaryPath[256];
		OutputBuffer[0] = 0;

		ConstructionContext ConsContext(SourceBaseName,
			OpenIntermediateFile,
			CloseIntermediateFile,
			DeleteIntermediateFile,
			OutputBuffer, sizeof(OutputBuffer),
			OutputBuffer, sizeof(OutputBuffer));
		GGenerator->Build(ConsContext, BinaryPath, sizeof(BinaryPath));

		Debugf("Construction finished with %s (arg %d) "
			"in stage %s, output saved to %s\n"
			"Spawn offset: 0x%08X Process offset: 0x%08X\n"
			"== Toolchain log starts here ==\n"
			"%s\n"
			"== Toolchain log ends here ==\n",
			ConsContext.GetResultString(), ConsContext.ResultArgument,
			ConsContext.GetStageString(), BinaryPath,
			ConsContext.SpawnCodeOffset, ConsContext.ProcessCodeOffset,
			OutputBuffer);

		size_t BlobSize;
		const void *Ptr = OpenIntermediateFile(BinaryPath, FAM_Read, &BlobSize);
		if (!Ptr)
		{
			Debugf("Failed to read the compiled binary file %s\n", BinaryPath);
			return 0;
		}
		if (!GLauncher->LoadRawBinary(emitter, Ptr, BlobSize,
			ConsContext.DataOffset, ConsContext.SpawnCodeOffset,
			ConsContext.ProcessCodeOffset))
		{
			Debugf("Failed to load the compiled binary: %p, %p, %p\n",
				emitter->InternalPtr1, emitter->InternalPtr2,
				emitter->InternalPtr3);
			CloseIntermediateFile(Ptr);
			return 0;
		}
		CloseIntermediateFile(Ptr);
	}

	return Result;
}

PTC_ATTRIBS size_t EXPORTDECL ptcProcessEmitter(ptcEmitter *emitter, float step,
		ptcVector cameraCS[3], ptcVertex *buffer, size_t maxVertices)
{
	uint32_t count;

	emitter->SpawnTimer += step;
	count = (size_t)floorf(emitter->SpawnTimer * emitter->Config.SpawnRate);
	emitter->SpawnTimer -= (float)count / emitter->Config.SpawnRate;
	count *= emitter->Config.BurstCount;
	if (count > emitter->MaxParticles - emitter->NumParticles)
		count = emitter->MaxParticles - emitter->NumParticles;
	if (count > 0) {
		//Debugf("libparticlasm: spawning a burst of %d particles\n", count);
		GLauncher->SpawnParticles(emitter, step, count);
	}

	// call the processing code
	count = GLauncher->ProcessParticles(emitter,
		emitter->ParticleBuf, emitter->ParticleBuf + emitter->MaxParticles,
		step, cameraCS, buffer, maxVertices);

	return count;
}

PTC_ATTRIBS void ptcReleaseEmitter(ptcEmitter *emitter)
{
	GLauncher->Unload(emitter);
}

/// \sa PFNGETPTCAPI
extern "C" EXPORTDECL uint32_t ptcGetAPI(uint32_t version,
	ptcAPIExports *API)
{
	if (version == PTC_API_VERSION)
	{
		API->QueryTargetSupport = ptcQueryTargetSupport;
		API->InitializeTarget = ptcInitializeTarget;
		API->ShutdownTarget = ptcShutdownTarget;
		API->CompileEmitter = ptcCompileEmitter;
		API->ProcessEmitter = ptcProcessEmitter;
		API->ReleaseEmitter = ptcReleaseEmitter;
		return 1;
	}
	return 0;
}

#ifndef NDEBUG
	SUPPRESS_WARNING_GCC_END
#endif
