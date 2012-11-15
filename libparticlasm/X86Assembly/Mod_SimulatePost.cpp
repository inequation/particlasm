/*
Particlasm x86 assembly generator module for simulation postprocessing
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Mod_SimulatePost.h"
#include "AsmSnippets/AsmSnippets.h"

Mod_SimulatePost::Mod_SimulatePost() :
	X86Module((ptcModuleID)-2)
{
	//ctor
}

Mod_SimulatePost::~Mod_SimulatePost()
{
	//dtor
}

void Mod_SimulatePost::Generate(CodeGenerationContext &Context,
			const ptcModule *Module) const
{
	assert(Context.Stage == GS_ProcessCode);
	assert(Module == NULL);
	Context.Emitf(Asm_Mod_SimulatePost);
	Context.Result = GR_Success;
}
