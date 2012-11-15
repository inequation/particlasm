/*
Particlasm x86 assembly generator
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include <cassert>
#include "X86AssemblyGenerator.h"

// include the assembly code so that it may be linked in
#define ASMSNIPPETS_DEFINITIONS
#include "AsmSnippets/AsmSnippets.h"

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
	EPlatform InPlatform) :
	Arch(InArch),
	Platform(InPlatform)
{
	//ctor
}

X86AssemblyGenerator::~X86AssemblyGenerator()
{
	//dtor
}

#define ARRAY_COUNT(a)	(sizeof(a) / sizeof((a)[0]))

void X86AssemblyGenerator::Generate(CodeGenerationContext& Context)
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
	Context.Emitf(Asm_Prologue, Inc_nasmx, Inc_libparticlasm);

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
		Context.Emitf("__%s:\n", StageString);

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
				if (Context.Result == GR_ModuleIDMismatch)
				{
					Context.Result = GR_Success;
					continue;
				}
				else if (Context.Result != GR_Success)
					return;
				// break the loop upon success
				break;
			}
			if (i >= ARRAY_COUNT(ModuleMap))
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
