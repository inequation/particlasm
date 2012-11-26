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
#include <ctime>

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
#include "../X86Assembly/X86AssemblyGenerator.h"
#include "../X86Assembly/X86Launcher.h"

FILE *GSourceCode;
const LauncherInterface *GLauncher = NULL;

static void CodeEmitf(const char *fmt, ...)
{
	va_list argptr;

	va_start(argptr, fmt);
	vfprintf(GSourceCode, fmt, argptr);
	va_end(argptr);
}

typedef std::map<void *, std::pair<int, size_t> > FileMap;
FileMap IntermediateFiles;

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
	// TODO
	switch (target)
	{
		case ptcTarget_x86_Linux:
		case ptcTarget_x86_64_Linux:
		case ptcTarget_x86_Windows:
		case ptcTarget_x86_64_Windows:
			return 1;
	}
	return 0;
}

PTC_ATTRIBS uint32_t ptcInitializeTarget(ptcTarget target, void *privateData)
{
	// TODO
	switch (target)
	{
		case ptcTarget_x86_Linux:
		case ptcTarget_x86_64_Linux:
		case ptcTarget_x86_Windows:
		case ptcTarget_x86_64_Windows:
			if (!GLauncher)
				GLauncher = new X86Launcher();
			return 1;
	}
	return 0;
}

PTC_ATTRIBS void ptcShutdownTarget(ptcTarget target, void *privateData)
{
	// TODO
	if (GLauncher)
	{
		delete GLauncher;
		GLauncher = NULL;
	}
}

PTC_ATTRIBS uint32_t ptcCompileEmitter(ptcEmitter *emitter)
{
	static char CodeFileName[256] = {0};

	snprintf(CodeFileName, sizeof(CodeFileName) - 1, "%sparticlasm_%d_%d",
#if defined(WIN32) || defined(__WIN32__)
		"%TEMP%\\", (int)GetCurrentProcessId(),
#else
		"/tmp/", getpid(),
#endif // WIN32
		(int)time(NULL) ^ rand());

	X86AssemblyGenerator Generator(X86AssemblyGenerator::ARCH_x86,
		X86AssemblyGenerator::PLATFORM_Linux,
		CodeFileName, sizeof(CodeFileName));

	GSourceCode = fopen(CodeFileName, "w");

	CodeGenerationContext GenContext(emitter, CodeEmitf);

	Generator.Generate(GenContext);
	printf("Generation finished with %s (arg %d) in module #%d in stage %s\n",
		GenContext.GetResultString(), GenContext.ResultArgument,
		GenContext.CurrentModuleIndex, GenContext.GetStageString());

	fclose(GSourceCode);

	if (GenContext.Result == GR_Success)
	{
		static char OutputBuffer[16384], BinaryPath[256];
		OutputBuffer[0] = 0;

		ConstructionContext ConsContext(CodeFileName,
			OpenIntermediateFile,
			CloseIntermediateFile,
			DeleteIntermediateFile,
			OutputBuffer, sizeof(OutputBuffer),
			OutputBuffer, sizeof(OutputBuffer));
		Generator.Build(ConsContext, BinaryPath, sizeof(BinaryPath));

		size_t BlobSize;
		GLauncher->LoadRawBinary(emitter,
			OpenIntermediateFile(BinaryPath, FAM_Read, &BlobSize), BlobSize,
			ConsContext.DataOffset,
			ConsContext.SpawnCodeOffset,
			ConsContext.ProcessCodeOffset);

		printf("Construction finished with %s (arg %d) "
			"in stage %s, output saved to %s\n"
			"Spawn offset: 0x%08X Process offset: 0x%08X\n"
			"== Toolchain log starts here ==\n"
			"%s\n"
			"== Toolchain log ends here ==\n",
			ConsContext.GetResultString(), ConsContext.ResultArgument,
			ConsContext.GetStageString(), BinaryPath,
			ConsContext.SpawnCodeOffset, ConsContext.ProcessCodeOffset,
			OutputBuffer);
	}

#ifdef NDEBUG
	remove(CodeFileName);
#endif

	return 0;
}

PTC_ATTRIBS size_t EXPORTDECL ptcProcessEmitter(ptcEmitter *emitter, float step,
		ptcVector cameraCS[3], ptcVertex *buffer, size_t maxVertices)
{
	return 0;
}

PTC_ATTRIBS void ptcReleaseEmitter(ptcEmitter *emitter)
{
}

/// \sa PFNGETPTCAPI
extern "C" EXPORTDECL uint32_t ptcGetAPI(uint32_t version,
	ptcAPIExports *API)
{
	// FIXME!!!
	//if (version == PTC_VERSION)
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
