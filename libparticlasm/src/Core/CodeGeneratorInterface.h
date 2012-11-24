/*
Particlasm code generator interface
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <cassert>
#include <libparticlasm2.h>

// ============================================================================
// Code generation context
// ============================================================================

typedef enum
{
	GR_Success,
	GR_InvalidEmitter,
	GR_InvalidCallback,
	GR_ModuleIDMismatch,
	GR_UnsupportedModuleID,
	GR_DistributionIDMismatch,
	GR_UnsupportedDistributionID
}
GenerationResult;

typedef enum
{
	GS_Started,
	GS_Data,
	GS_SpawnCode,
	GS_ProcessCode,
	GS_Finished
}
GenerationStage;

typedef PTC_ATTRIBS void (* PFNBUFFERPRINTF)(const char *fmt, ...);

struct CodeGenerationContext
{
	ptcEmitter				*Emitter;
	PFNBUFFERPRINTF			Emitf;

	int						CurrentModuleIndex;
	int						CurrentDataIndex;
	GenerationStage			Stage;
	GenerationResult		Result;
	int						ResultArgument;

	CodeGenerationContext(ptcEmitter *InEmitter, PFNBUFFERPRINTF InEmitf)
		:
		Emitter(InEmitter),
		Emitf(InEmitf),
		CurrentModuleIndex(-1),
		CurrentDataIndex(-1),
		Stage(GS_Started),
		Result(GR_Success),
		ResultArgument(0)
	{
		assert(Emitf);
	}

#define CG_ENUM_STR(x)	case x:	return #x; break
	const char *GetStageString()
	{
		switch (Stage)
		{
			CG_ENUM_STR(GS_Started);
			CG_ENUM_STR(GS_Data);
			CG_ENUM_STR(GS_SpawnCode);
			CG_ENUM_STR(GS_ProcessCode);
			CG_ENUM_STR(GS_Finished);
			default:	assert(!"Unknown stage"); return "unknown"; break;
		}
	}

	const char *GetResultString()
	{
		switch (Result)
		{
			CG_ENUM_STR(GR_Success);
			CG_ENUM_STR(GR_InvalidEmitter);
			CG_ENUM_STR(GR_InvalidCallback);
			CG_ENUM_STR(GR_ModuleIDMismatch);
			CG_ENUM_STR(GR_UnsupportedModuleID);
			CG_ENUM_STR(GR_DistributionIDMismatch);
			CG_ENUM_STR(GR_UnsupportedDistributionID);
			default:	assert(!"Unknown result"); return "unknown"; break;
		}
	}
#undef CG_ENUM_STR
};

// ============================================================================
// Construction context
// ============================================================================

typedef enum
{
	CR_Success,
	CR_InvalidFileName,
	CR_ToolchainSpawningFailure,
	CR_ToolchainError,
	CR_IntermediateFileAccessFailure,
	CR_CorruptIntermediateFile
}
ConstructionResult;

typedef enum
{
	CS_Started,
	CS_Compiling,
	CS_Linking,
	CS_Finished
}
ConstructionStage;

typedef enum
{
	FAM_Read		= 0x01,
	FAM_Write		= 0x02,
	FAM_ReadWrite	= FAM_Read | FAM_Write,

	FAM_PADDING		= 0xFFFFFFFF	// dummy to ensure 32 bit enum width
}
FileAccessMode;

typedef PTC_ATTRIBS const void *(* PFNOPENINTERMEDIATEFILE)(const char *Path,
	FileAccessMode Mode, size_t *OutFileSize);
typedef PTC_ATTRIBS void (* PFNCLOSEINTERMEDIATEFILE)(const void *FilePtr);
typedef PTC_ATTRIBS void (* PFNDELETEINTERMEDIATEFILE)(const char *Path);
typedef PTC_ATTRIBS void (* PFNLOADBINARYFILE)(const char *Path);

struct ConstructionContext
{
	char					*FileName;
	char					*StdoutBuffer;
	size_t					StdoutBufferSize;
	char					*StderrBuffer;
	size_t					StderrBufferSize;

	PFNOPENINTERMEDIATEFILE	OpenIntermediateFile;
	PFNCLOSEINTERMEDIATEFILE	CloseIntermediateFile;
	PFNDELETEINTERMEDIATEFILE	DeleteIntermediateFile;
	PFNLOADBINARYFILE		LoadBinaryFile;

	ConstructionStage		Stage;
	ConstructionResult		Result;
	int						ResultArgument;
	size_t					SpawnCodeOffset;
	size_t					ProcessCodeOffset;

	ConstructionContext(char *InFileName,
		PFNOPENINTERMEDIATEFILE InOpenIntermediateFile,
		PFNCLOSEINTERMEDIATEFILE InCloseIntermediateFile,
		PFNDELETEINTERMEDIATEFILE InDeleteIntermediateFile,
		PFNLOADBINARYFILE InLoadBinaryFile,
		char *InStdoutBuffer = NULL, size_t InStdoutBufferSize = 0,
		char *InStderrBuffer = NULL, size_t InStderrBufferSize = 0)
		:
		FileName(InFileName),
		StdoutBuffer(InStdoutBuffer),
		StdoutBufferSize(InStdoutBufferSize),
		StderrBuffer(InStderrBuffer),
		StderrBufferSize(InStderrBufferSize),
		OpenIntermediateFile(InOpenIntermediateFile),
		CloseIntermediateFile(InCloseIntermediateFile),
		DeleteIntermediateFile(InDeleteIntermediateFile),
		LoadBinaryFile(InLoadBinaryFile),
		Stage(CS_Started),
		Result(CR_Success)
	{
		assert(OpenIntermediateFile);
		assert(CloseIntermediateFile);
		assert(DeleteIntermediateFile);
		assert(LoadBinaryFile);
	}

#define CC_ENUM_STR(x)	case x:	return #x; break
	const char *GetStageString()
	{
		switch (Stage)
		{
			CC_ENUM_STR(CS_Started);
			CC_ENUM_STR(CS_Compiling);
			CC_ENUM_STR(CS_Linking);
			CC_ENUM_STR(CS_Finished);
			default:	assert(!"Unknown stage"); return "unknown"; break;
		}
	}

	const char *GetResultString()
	{
		switch (Result)
		{
			CC_ENUM_STR(CR_Success);
			CC_ENUM_STR(CR_InvalidFileName);
			CC_ENUM_STR(CR_ToolchainSpawningFailure);
			CC_ENUM_STR(CR_ToolchainError);
			CC_ENUM_STR(CR_IntermediateFileAccessFailure);
			CC_ENUM_STR(CR_CorruptIntermediateFile);
			default:	assert(!"Unknown result"); return "unknown"; break;
		}
	}
#undef CC_ENUM_STR
};

class CodeGeneratorInterface
{
	public:
		virtual void Generate(CodeGenerationContext& Context) const = 0;
		virtual void Build(ConstructionContext& Context) const = 0;

	protected:
		bool RunProcess(const char *CommandLine, int& OutExitCode,
			char *StdoutBuffer = NULL, size_t StdoutBufferSize = 0,
			char *StderrBuffer = NULL, size_t StderrBufferSize = 0) const;
};

#endif // CODEGENERATOR_H
