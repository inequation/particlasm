/*
Particlasm x86 assembly generator module for simulation preprocessing
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include <cassert>
#include "Mod_SimulatePre.h"
#include "AsmSnippets.h"

Mod_SimulatePre::Mod_SimulatePre() :
	X86Module((ptcModuleID)-1)
{
	//ctor
}

void Mod_SimulatePre::Generate(CodeGenerationContext &Context,
			const ptcModule *Module) const
{
	assert(Context.Stage == GS_ProcessCode);
	assert(Module == NULL);
	Context.Emitf(Asm_Mod_SimulatePre);
	Context.Result = GR_Success;
}
