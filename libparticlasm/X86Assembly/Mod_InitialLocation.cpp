/*
Particlasm x86 assembly generator module for initial location
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Mod_InitialLocation.h"

Mod_InitialLocation::Mod_InitialLocation() :
	X86Module(ptcMID_InitialLocation)
{
	//ctor
}

Mod_InitialLocation::~Mod_InitialLocation()
{
	//dtor
}

void Mod_InitialLocation::Generate(CodeGenerationContext& Context,
	const ptcModule *Module) const
{
	X86Module::Generate(Context, Module);
	if (Context.Result != GR_Success)
		return;
}
