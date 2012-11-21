/*
Particlasm x86 assembly generator module for gravity
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include <cassert>
#include "Mod_Gravity.h"
#include "AsmSnippets.h"

Mod_Gravity::Mod_Gravity() :
	X86ModuleInterface(ptcMID_Gravity)
{
	//ctor
}

void Mod_Gravity::Generate(CodeGenerationContext& Context,
	const ptcModule *Module) const
{
	X86ModuleInterface::Generate(Context, Module);
	if (Context.Result != GR_Success)
		return;
	assert(!"TODO");
}
