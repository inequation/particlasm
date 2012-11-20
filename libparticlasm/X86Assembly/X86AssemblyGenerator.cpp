/*
Particlasm x86 assembly generator
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include <cassert>
#include <cstring>
#include <cstdio>
#include "X86AssemblyGenerator.h"

// include the assembly code so that it may be linked in
#define ASMSNIPPETS_DEFINITIONS
#include "AsmSnippets.h"

const Mod_SimulatePre X86AssemblyGenerator::SimPre;
const Mod_SimulatePost X86AssemblyGenerator::SimPost;
const X86Module *X86AssemblyGenerator::ModuleMap[] =
{
	new Mod_InitialLocation(),
	new Mod_InitialRotation(),
	new Mod_InitialSize(),
	new Mod_InitialVelocity(),
	new Mod_InitialColour(),
	new Mod_Velocity(),
	new Mod_Acceleration(),
	new Mod_Colour(),
	new Mod_Size(),
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
		return;
	}
	if (!Context.Emitf)
	{
		Context.Result = GR_InvalidCallback;
		return;
	}

	// start off by integrating the prologue
	Context.Emitf(Asm_Prologue,
		Arch == ARCH_x86 ? "P3" : "X64",	// CPU
		Arch == ARCH_x86 ? "32" : "64",		// BITS
		Inc_nasmx,							// nasmx
		Inc_libparticlasm);					// libparticlasm header

	// perform the generation stages
	for (Context.Stage = GS_Data;
		Context.Stage < GS_Finished;
		Context.Stage = (GenerationStage)((int)Context.Stage + 1))
	{
		Context.CurrentModuleIndex = 0;
		Context.CurrentDataIndex = 0;

		const char *StageString;
		switch (Context.Stage)
		{
			case GS_Data:
				StageString = "Data";
				break;
			case GS_SpawnCode:
				StageString = "Spawn";
				break;
			case GS_ProcessCode:
				StageString = "Process";
				break;
			default:
				assert(Context.Stage > GS_Started
					&& Context.Stage < GS_Finished);
		}
		// print out the label
		Context.Emitf("\n__%s:\n", StageString);

		if (Context.Stage == GS_ProcessCode)
		{
			// special case: pre-simulation
			SimPre.Generate(Context, NULL);
			if (Context.Result != GR_Success)
				return;
		}

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
				Context.Result = GR_UnsupportedModuleID;

			if (Context.Result != GR_Success)
				return;
		}

		if (Context.Stage == GS_ProcessCode)
		{
			// special case: post-simulation
			SimPost.Generate(Context, NULL);
			if (Context.Result != GR_Success)
				return;
		}
	}

	// finish off by integrating the epilogue
	Context.Emitf(Asm_Epilogue);
}

void X86AssemblyGenerator::Build(ConstructionContext& Context) const
{
	// some sanity checking
	if (!Context.FileName)
	{
		Context.Result = CR_InvalidFileName;
		return;
	}

	Context.Stage = CS_Compiling;

	char CmdLine[256];
	snprintf(CmdLine, sizeof(CmdLine) - 1, "nasm -f bin %s", Context.FileName);
	CmdLine[sizeof(CmdLine) - 1] = 0;

	if (!RunProcess(CmdLine, Context.ToolchainExitCode,
		Context.StdoutBuffer, Context.StdoutBufferSize,
		Context.StderrBuffer, Context.StderrBufferSize))
	{
		Context.Result = CR_ToolchainSpawningFailure;
		return;
	}

	if (Context.ToolchainExitCode == 0)
	{
		Context.Result = CR_Success;
		Context.Stage = CS_Finished;
	}
	else
		Context.Result = CR_ToolchainError;
}
