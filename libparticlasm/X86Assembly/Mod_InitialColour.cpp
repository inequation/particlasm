/*
Particlasm x86 assembly generator module for initial colour
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Mod_InitialColour.h"

Mod_InitialColour::Mod_InitialColour() :
	X86Module(ptcMID_InitialColour)
{
	//ctor
}

Mod_InitialColour::~Mod_InitialColour()
{
	//dtor
}

void Mod_InitialColour::Generate(CodeGenerationContext& Context,
	const ptcModule *Module) const
{
	X86Module::Generate(Context, Module);
	if (Context.Result != GR_Success)
		return;
}
