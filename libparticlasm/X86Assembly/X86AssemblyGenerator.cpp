/*
Particlasm x86 assembly generator
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include <cassert>
#include "X86AssemblyGenerator.h"
#include "X86Module.h"
#include "Mod_SimulatePre.h"
#include "Mod_SimulatePost.h"
#include "Mod_InitialLocation.h"

// include the assembly code so that it may be linked in
#define ASMSNIPPETS_DEFINITIONS
#include "AsmSnippets/AsmSnippets.h"

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

static const Mod_SimulatePre SimPre;
static const Mod_SimulatePost SimPost;
static const X86Module ModuleMap[] =
{
	Mod_InitialLocation()
};

#define ARRAY_COUNT(a)	(sizeof(a) / sizeof((a)[0]))

void X86AssemblyGenerator::Generate(CodeGenerationContext& Context)
{
	// some sanity checking
	if (!Context.Emitter)
	{
		Context.Result = GR_InvalidEmitter;
		return;
	}
	if (!Context.Printf)
	{
		Context.Result = GR_InvalidCallback;
		return;
	}

	// start off by integrating the prologue
	Context.Printf(Prologue);

	// perform the generation stages
	for (Context.Stage = GS_Data;
		Context.Stage < GS_Finished;
		Context.Stage = (GenerationStage)((int)Context.Stage + 1))
	{
		Context.CurrentModuleIndex = 0;

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
				// special case: pre-simulation
				SimPre.Generate(Context, NULL);
				if (Context.Result != GR_Success)
					return;
				break;
			default:
				assert(Context.Stage > GS_Started
					&& Context.Stage < GS_Finished);
		}
		// print out the label
		Context.Printf("__%s:\n", StageString);

		// walk the module list
		for (ptcModule *Mod = Context.Emitter->Head;
			Mod;
			Mod = Mod->Header.Next, ++Context.CurrentModuleIndex)
		{
			size_t i;
			for (i = 0; i < ARRAY_COUNT(ModuleMap); ++i)
			{
				ModuleMap[i].Generate(Context, Mod);
				if (Context.Result == GR_ModuleIDMismatch)
				{
					Context.Result = GR_Success;
					continue;
				}
				else if (Context.Result != GR_Success)
					return;
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
	Context.Printf(Epilogue);
}
