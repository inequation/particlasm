/*
Particlasm x86 assembly generator module for simulation postprocessing
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "../Core/ParticlasmMain.h"
#include "Mod_SimulatePost.h"
#include "AsmSnippets.h"

Mod_SimulatePost::Mod_SimulatePost() :
	X86ModuleInterface((ptcModuleID)-2)
{
	//ctor
}

SUPPRESS_WARNING_GCC_BEGIN("-Wunused-parameter")

void Mod_SimulatePost::Generate(CodeGenerationContext &Context,
			const ptcModule *Module) const
{
	assert(Context.Stage == GS_ProcessCode);
	assert(Module == NULL);
	Context.Emitf(Asm_Mod_SimulatePost);
	Context.Result = GR_Success;
	Context.ResultArgument = 0;
}

SUPPRESS_WARNING_END
