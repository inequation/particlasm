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

typedef PTC_ATTRIBS void (* PFNOPENSOURCEFILE)(const char *Path);
typedef PTC_ATTRIBS void (* PFNSOURCEEMITF)(const char *fmt, ...);
typedef PTC_ATTRIBS void (* PFNCLOSESOURCEFILE)();

struct CodeGenerationContext
{
	PFNOPENSOURCEFILE		OpenSourceFile;
	PFNSOURCEEMITF			Emitf;
	PFNCLOSESOURCEFILE		CloseSourceFile;

	ptcEmitter				*Emitter;
	const char				*SourceFileBaseName;

	int						CurrentModuleIndex;
	GenerationStage			Stage;
	GenerationResult		Result;
	int						ResultArgument;

	void					*PrivateData;

	CodeGenerationContext(ptcEmitter *InEmitter,
		const char *InSourceFileBaseName, PFNOPENSOURCEFILE InOpenSourceFile,
		PFNSOURCEEMITF InEmitf, PFNCLOSESOURCEFILE InCloseSourceFile)
		:
		OpenSourceFile(InOpenSourceFile),
		Emitf(InEmitf),
		CloseSourceFile(InCloseSourceFile),
		Emitter(InEmitter),
		SourceFileBaseName(InSourceFileBaseName),
		CurrentModuleIndex(-1),
		Stage(GS_Started),
		Result(GR_Success),
		ResultArgument(0),
		PrivateData(NULL)
	{
		assert(OpenSourceFile);
		assert(Emitf);
		assert(CloseSourceFile);
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

struct ConstructionContext
{
	char					*SourceBaseFileName;
	char					*StdoutBuffer;
	size_t					StdoutBufferSize;
	char					*StderrBuffer;
	size_t					StderrBufferSize;

	PFNOPENINTERMEDIATEFILE	OpenIntermediateFile;
	PFNCLOSEINTERMEDIATEFILE	CloseIntermediateFile;
	PFNDELETEINTERMEDIATEFILE	DeleteIntermediateFile;

	ConstructionStage		Stage;
	ConstructionResult		Result;
	int						ResultArgument;
	size_t					DataOffset;
	size_t					SpawnCodeOffset;
	size_t					ProcessCodeOffset;

	void					*PrivateData;

	ConstructionContext(char *InSourceBaseFileName,
		PFNOPENINTERMEDIATEFILE InOpenIntermediateFile,
		PFNCLOSEINTERMEDIATEFILE InCloseIntermediateFile,
		PFNDELETEINTERMEDIATEFILE InDeleteIntermediateFile,
		char *InStdoutBuffer = NULL, size_t InStdoutBufferSize = 0,
		char *InStderrBuffer = NULL, size_t InStderrBufferSize = 0)
		:
		SourceBaseFileName(InSourceBaseFileName),
		StdoutBuffer(InStdoutBuffer),
		StdoutBufferSize(InStdoutBufferSize),
		StderrBuffer(InStderrBuffer),
		StderrBufferSize(InStderrBufferSize),
		OpenIntermediateFile(InOpenIntermediateFile),
		CloseIntermediateFile(InCloseIntermediateFile),
		DeleteIntermediateFile(InDeleteIntermediateFile),
		Stage(CS_Started),
		Result(CR_Success),
		PrivateData(NULL)
	{
		assert(OpenIntermediateFile);
		assert(CloseIntermediateFile);
		assert(DeleteIntermediateFile);
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
		virtual ~CodeGeneratorInterface() {}

		virtual void Generate(CodeGenerationContext& Context) const = 0;
		virtual void Build(ConstructionContext& Context, char *OutBinaryPath,
			size_t OutBinaryPathSize) const = 0;

	protected:
		bool RunProcess(const char *CommandLine, int& OutExitCode,
			char *StdoutBuffer = NULL, size_t StdoutBufferSize = 0,
			char *StderrBuffer = NULL, size_t StderrBufferSize = 0) const;
};

#endif // CODEGENERATOR_H
