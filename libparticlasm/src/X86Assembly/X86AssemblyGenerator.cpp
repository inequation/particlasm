/*
Particlasm x86 assembly generator
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include <cassert>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include "X86AssemblyGenerator.h"
#include "X86Assembly.h"
#include "X86ModuleInterface.h"
#include "X86SimpleModule.h"
#include "Mod_SimulatePre.h"
#include "Mod_SimulatePost.h"
#include "Mod_InitialColour.h"
#include "Mod_Colour.h"
#include "Mod_Gravity.h"

// include the assembly code so that it may be linked in
#define ASMSNIPPETS_DEFINITIONS
#include "AsmSnippets.h"

#define SM_TEMPL(Type)	<Type##Distr>
#define NEW_SM_EX(Name, Type, Val, SpawnPre, SpawnPost, ProcPre, ProcPost) \
	new X86SimpleModule SM_TEMPL(Type) (ptcMID_##Name, SpawnPost, ProcPost, \
		offsetof(ptcMod_##Name, Val), SpawnPre, ProcPre)
#define NEW_SM(Name, Type, Val, SpawnPost, ProcPost) \
	NEW_SM_EX(Name, Type, Val, NULL, SpawnPost, NULL, ProcPost)

const Mod_SimulatePre X86AssemblyGenerator::SimPre;
const Mod_SimulatePost X86AssemblyGenerator::SimPost;
const X86ModuleInterface *X86AssemblyGenerator::ModuleMap[] =
{
	NEW_SM(InitialLocation, ptcVector, Distr, Asm_Mod_InitialLocation, NULL),
	NEW_SM(InitialRotation, ptcScalar, Distr, Asm_Mod_InitialRotation, NULL),
	NEW_SM(InitialSize, ptcScalar, Distr, Asm_Mod_InitialSize, NULL),
	NEW_SM(InitialVelocity, ptcVector, Distr, Asm_Mod_InitialVelocity, NULL),
	NEW_SM(Velocity, ptcVector, Distr, NULL, Asm_Mod_Velocity),
	NEW_SM(Acceleration, ptcVector, Distr, NULL, Asm_Mod_Acceleration),
	NEW_SM(Size, ptcScalar, Distr, NULL, Asm_Mod_Size),
	new Mod_InitialColour(),
	new Mod_Colour(),
	new Mod_Gravity()
};

X86AssemblyGenerator::X86AssemblyGenerator(EArchitecture InArch,
	EPlatform InPlatform, char *CodeFileName, size_t CodeFileNameSize) :
	Arch(InArch),
	Platform(InPlatform)
{
	strncat(CodeFileName, ".asm", CodeFileNameSize - strlen(CodeFileName));
}

#define ARRAY_COUNT(a)	(sizeof(a) / sizeof((a)[0]))

void X86AssemblyGenerator::Generate(CodeGenerationContext& Context) const
{
	// some sanity checking
	if (!Context.Emitter)
	{
		Context.Result = GR_InvalidEmitter;
		Context.ResultArgument = 0;
		return;
	}
	if (!Context.Emitf)
	{
		Context.Result = GR_InvalidCallback;
		Context.ResultArgument = 0;
		return;
	}

	// create a private context data instance
	PrivateContextData PCD;
	Context.PrivateData = &PCD;

	// start off by integrating the prologue
	const char *CPU, *BITS, *OUTPUT_FORMAT;
	switch (Arch)
	{
		case ARCH_x86:
			CPU = "P3";
			BITS = "32";
			OUTPUT_FORMAT = Platform == PLATFORM_Windows ? "win32" : "elf32";
			break;
		case ARCH_x86_64:
			CPU = "X64";
			BITS = "64";
			OUTPUT_FORMAT = Platform == PLATFORM_Windows ? "win64" : "elf64";
			break;
		default:
			assert(!"Invalid architecture or platform");
	}
	Context.Emitf(Asm_Prologue,
		CPU, BITS, OUTPUT_FORMAT,
		Inc_nasmx,
		Inc_libparticlasm);

	// perform the generation stages
	for (Context.Stage = GS_Data;
		Context.Stage < GS_Finished;
		Context.Stage = (GenerationStage)((int)Context.Stage + 1))
	{
		// print out the label
		switch (Context.Stage)
		{
			case GS_Data:
				Context.Emitf("\n__Data:\n");
				break;
			case GS_SpawnCode:
				Context.Emitf("\nproc __Spawn\n"
								"locals none\n");
				break;
			case GS_ProcessCode:
				Context.Emitf("\nproc __Process\n"
								"locals none\n");
				break;
			default:
				assert(Context.Stage > GS_Started
					&& Context.Stage < GS_Finished);
		}

		if (Context.Stage == GS_ProcessCode)
		{
			// special case: pre-simulation
			Context.CurrentModuleIndex = -1;
			SimPre.Generate(Context, NULL);
			if (Context.Result != GR_Success)
				return;
		}
		Context.CurrentModuleIndex = 0;

		// walk the module list
		for (ptcModule *Mod = Context.Emitter->Head;
			Mod;
			Mod = Mod->Header.Next, ++Context.CurrentModuleIndex)
		{
			size_t i;
			for (i = 0; i < ARRAY_COUNT(ModuleMap); ++i)
			{
				ModuleMap[i]->Generate(Context, Mod);
				if (Context.Result == GR_Success)
					// break the loop upon success
					break;
				else if (Context.Result == GR_ModuleIDMismatch)
					// try another module
					continue;
				else
					return;
			}
			if (i >= ARRAY_COUNT(ModuleMap) && Context.Result != GR_Success)
			{
				Context.Result = GR_UnsupportedModuleID;
				Context.ResultArgument = Mod->Header.ModuleID;
			}

			if (Context.Result != GR_Success)
				return;
		}

		if (Context.Stage == GS_ProcessCode)
		{
			// special case: post-simulation
			Context.CurrentModuleIndex = -2;
			SimPost.Generate(Context, NULL);
			if (Context.Result != GR_Success)
				return;
			Context.Emitf("endproc\n");
		}
		else if (Context.Stage == GS_SpawnCode)
		{
			// special case: procedure end
			Context.Emitf("endproc\n");
		}
	}

	// clear the private data pointer
	Context.PrivateData = NULL;

	// finish off by integrating the epilogue
	Context.Emitf(Asm_Epilogue);
}

void X86AssemblyGenerator::Build(ConstructionContext& Context,
	char *OutBinaryPath, size_t OutBinaryPathSize) const
{
	// some sanity checking
	if (!Context.FileName)
	{
		Context.Result = CR_InvalidFileName;
		Context.ResultArgument = 0;
		return;
	}

	Context.Stage = CS_Compiling;

	char CmdLine[256], ListFile[256];

	// determine a path to the listing file
	strncpy(ListFile, Context.FileName, sizeof(ListFile) - 5);
	ListFile[sizeof(ListFile) - 5] = 0;
	char *Extension = strrchr(ListFile, '.');
	if (!Extension)
		Extension = ListFile + strlen(ListFile);
	strcpy(Extension, ".lst");

	snprintf(CmdLine, sizeof(CmdLine) - 1, "nasm -f bin -l %s %s",
		ListFile, Context.FileName);
	CmdLine[sizeof(CmdLine) - 1] = 0;

	if (!RunProcess(CmdLine, Context.ResultArgument,
		Context.StdoutBuffer, Context.StdoutBufferSize,
		Context.StderrBuffer, Context.StderrBufferSize))
	{
		Context.Result = CR_ToolchainSpawningFailure;
		Context.ResultArgument = 0;
		return;
	}

	if (Context.ResultArgument == 0)
	{
		FindCodeOffsets(Context, ListFile);
		if (Context.Result != CR_Success)
			return;

		const size_t BaseNameLength = std::min((size_t)(Extension - ListFile),
			OutBinaryPathSize - 1);
		strncpy(OutBinaryPath, Context.FileName, BaseNameLength);
		OutBinaryPath[BaseNameLength] = 0;

		Context.Result = CR_Success;
		Context.Stage = CS_Finished;
	}
	else
		Context.Result = CR_ToolchainError;

	Context.DeleteIntermediateFile(ListFile);
}

void X86AssemblyGenerator::FindCodeOffsets(ConstructionContext& Context,
	const char *ListFileName) const
{
	size_t FileSize;
	const char *Listing = (const char *)Context.OpenIntermediateFile(
		ListFileName, FAM_Read, &FileSize);
	if (!Listing)
	{
		Context.Result = CR_IntermediateFileAccessFailure;
		Context.ResultArgument = 0;
		return;
	}
	const char *End = Listing + FileSize;

	unsigned int Offset;
	char ProcName[32];
	bool FoundSpawn = false, FoundProcess = false;
	const char *LineEnd, *Line;
	for (Line = Listing;
		Line < End && (!FoundSpawn || !FoundProcess);
		Line = LineEnd + 1)
	{
		LineEnd = Line;
		while (LineEnd < End && *LineEnd != '\n')
			++LineEnd;
		const int Matches = sscanf(Line,
			"%*u %x %*x proc %32s",
			&Offset, ProcName);
		if (Matches != 2)
			continue;
		if (!strcmp(ProcName, "__Spawn"))
		{
			Context.SpawnCodeOffset = Offset;
			FoundSpawn = true;
		}
		else if (!strcmp(ProcName, "__Process"))
		{
			Context.ProcessCodeOffset = Offset;
			FoundProcess = true;
		}
	}
	// data is always at the start
	Context.DataOffset = 0;

	Context.CloseIntermediateFile(Listing);

	if (FoundSpawn && FoundProcess)
	{
		Context.Result = CR_Success;
		Context.ResultArgument = 0;
	}
	else
	{
		Context.Result = CR_CorruptIntermediateFile;
		// encode the cause in the lowest bits
		Context.ResultArgument = ((int)(!FoundSpawn)		* 1)
								| ((int)(!FoundProcess)		* 2)
								| ((int)(Line > End + 1)	* 4);
	}
}
