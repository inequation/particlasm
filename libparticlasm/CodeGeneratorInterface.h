/*
Particlasm code generator interface
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <cassert>
#include "libparticlasm.h"

typedef enum
{
	GR_Success,
	GR_InvalidEmitter,
	GR_InvalidCallback,
	GR_ModuleIDMismatch,
	GR_UnsupportedModuleID
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
	PFNBUFFERPRINTF			Printf;

	int						CurrentModuleIndex;
	GenerationStage			Stage;
	GenerationResult		Result;

	CodeGenerationContext(ptcEmitter *InEmitter, PFNBUFFERPRINTF InPrintf) :
		Emitter(InEmitter),
		Printf(InPrintf),
		CurrentModuleIndex(-1),
		Stage(GS_Started),
		Result(GR_Success)
		{}

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
			default:	return "unknown"; assert(!"Unknown stage"); break;
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
			default:	return "unknown"; assert(!"Unknown result"); break;
		}
	}
#undef CG_ENUM_STR
};

class CodeGeneratorInterface
{
	public:
		virtual void Generate(CodeGenerationContext& Context) = 0;
};

#endif // CODEGENERATOR_H
