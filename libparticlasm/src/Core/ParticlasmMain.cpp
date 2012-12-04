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
#include <cstdio>
#include <map>

#ifdef NDEBUG
	#define Debugf(Fmt, ...)	(void)0
#else
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

#if defined(WIN32) || defined(__WIN32__)
struct FileInfo
{
	HANDLE	File;
	HANDLE	Mapping;
	bool	NeedsFlush;

	FileInfo(HANDLE InFile, HANDLE InMapping, bool InNeedsFlush)
		: File(InFile), Mapping(InMapping), NeedsFlush(InNeedsFlush) {}
};
#else
struct FileInfo
{
	int		fd;
	size_t	size;

	FileInfo(int in_fd, size_t in_size)
		: fd(in_fd), size(in_size) {}
};
#endif

typedef std::map<void *, FileInfo> FileMap;
static FileMap IntermediateFiles;

static const void *OpenIntermediateFile(const char *Path, FileAccessMode Mode,
	size_t *OutFileSize)
{
#if defined(WIN32) || defined(__WIN32__)
	DWORD AccessFlags, ShareFlags, CreateFlags;
	switch (Mode & 0x03)
	{
		case FAM_Read:
			AccessFlags = GENERIC_READ;
			ShareFlags = FILE_SHARE_READ;
			CreateFlags = OPEN_EXISTING;
			break;
		case FAM_Write:
			AccessFlags = GENERIC_WRITE;
			ShareFlags = 0;
			CreateFlags = CREATE_ALWAYS;
			break;
		case FAM_ReadWrite:
			AccessFlags = GENERIC_READ | GENERIC_WRITE;
			ShareFlags = 0;
			CreateFlags = OPEN_ALWAYS;
			break;
		default:			return NULL;
	}

	HANDLE File = CreateFile(Path, AccessFlags, ShareFlags, NULL, CreateFlags,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (File == INVALID_HANDLE_VALUE)
		return NULL;

	LARGE_INTEGER FileSize;
	if (!GetFileSizeEx(File, &FileSize))
	{
		CloseHandle(File);
		return NULL;
	}
	if (OutFileSize)
		*OutFileSize = static_cast<size_t>(FileSize.QuadPart);

	LARGE_INTEGER MaxSize = {{0, 0}};
	DWORD ProtectionFlags;
	switch (Mode & 0x03)
	{
		case FAM_Read:
			MaxSize = FileSize;
			ProtectionFlags = PAGE_READONLY;
			AccessFlags = FILE_MAP_READ;
			break;
		case FAM_Write:
			MaxSize.QuadPart = (Mode & ~0x03) >> 2;
			ProtectionFlags = PAGE_READWRITE;
			AccessFlags = FILE_MAP_WRITE;
			break;
		case FAM_ReadWrite:
			MaxSize.QuadPart = FileSize.QuadPart * 2;
			ProtectionFlags = PAGE_READWRITE;
			AccessFlags = FILE_MAP_ALL_ACCESS;
			break;
		case FAM_PADDING:
			break;	// shut up compiler
	}
	HANDLE Mapping = CreateFileMapping(File, NULL, ProtectionFlags,
		MaxSize.u.HighPart, MaxSize.u.LowPart, NULL);
	if (Mapping == NULL)
	{
		CloseHandle(File);
		return NULL;
	}
	void *Ptr = MapViewOfFile(Mapping, AccessFlags, 0, 0, 0);
	if (Ptr == NULL)
	{
		CloseHandle(Mapping);
		CloseHandle(File);
		return NULL;
	}

	IntermediateFiles.insert(FileMap::value_type
		(Ptr, FileInfo(File, Mapping, (Mode & 0x03) != FAM_Read)));

	return Ptr;
#else // WIN32
	int FileFlags, ProtectionFlags;
	switch (Mode & 0x03)
	{
		case FAM_Read:
			FileFlags = O_RDONLY;
			ProtectionFlags = PROT_READ;
			break;
		case FAM_Write:
			FileFlags = O_WRONLY | O_CREAT | O_TRUNC;
			ProtectionFlags = PROT_WRITE;
			break;
		case FAM_ReadWrite:
			FileFlags = O_RDWR;
			ProtectionFlags = PROT_READ | PROT_WRITE;
			break;
		default:
			return NULL;
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

	size_t MappingLength = 0;
	switch(Mode & 0x03)
	{
		case FAM_Read:		MappingLength = FileStat.st_size;			break;
		case FAM_Write:		MappingLength = (Mode & ~0x03) >> 2;		break;
		case FAM_ReadWrite:	MappingLength = 2 * FileStat.st_size;		break;
		case FAM_PADDING:	break;	// shut up compiler
	}
	void *Ptr = mmap(NULL, MappingLength, ProtectionFlags,
		MAP_SHARED | MAP_FILE, fd, 0);
	if (Ptr == MAP_FAILED)
	{
		close(fd);
		return NULL;
	}

	IntermediateFiles.insert(FileMap::value_type
		(Ptr, FileInfo(fd, FileStat.st_size)));

	return Ptr;
#endif // WIN32
}

static void CloseIntermediateFile(const void *FilePtr)
{
	FileMap::iterator It = IntermediateFiles.find((void *)FilePtr);
	if (It == IntermediateFiles.end())
		return;
#if defined(WIN32) || defined(__WIN32__)
	if (It->second.NeedsFlush)
		FlushViewOfFile(FilePtr, 0);
	UnmapViewOfFile(FilePtr);
	CloseHandle(It->second.Mapping);
	CloseHandle(It->second.File);
#else // WIN32
	munmap(It->first, It->second.size);
	close(It->second.fd);
#endif // WIN32
	IntermediateFiles.erase(It);
}

// get rid of unused parameter warnings
SUPPRESS_WARNING_GCC_BEGIN("-Wunused-parameter")

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
			"Spawn offset: 0x%08zX Process offset: 0x%08zX\n"
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

		// huge success!
		CloseIntermediateFile(Ptr);
		emitter->NumParticles = 0;
	}

	return Result;
}

PTC_ATTRIBS uint32_t EXPORTDECL ptcProcessEmitter(ptcEmitter *emitter,
	float step, ptcVector cameraCS[3], ptcVertex *buffer, uint32_t maxVertices)
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

SUPPRESS_WARNING_GCC_END
