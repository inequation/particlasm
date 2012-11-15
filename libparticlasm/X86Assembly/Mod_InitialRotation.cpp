/*
Particlasm x86 assembly generator module for initial rotation
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Mod_InitialRotation.h"

Mod_InitialRotation::Mod_InitialRotation() :
	X86Module(ptcMID_InitialRotation)
{
	//ctor
}

Mod_InitialRotation::~Mod_InitialRotation()
{
	//dtor
}

void Mod_InitialRotation::Generate(CodeGenerationContext& Context,
	const ptcModule *Module) const
{
	X86Module::Generate(Context, Module);
	if (Context.Result != GR_Success)
		return;
}
