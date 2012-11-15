/*
Particlasm x86 assembly generator module for colour
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Mod_Colour.h"

Mod_Colour::Mod_Colour() :
	X86Module(ptcMID_Colour)
{
	//ctor
}

Mod_Colour::~Mod_Colour()
{
	//dtor
}

void Mod_Colour::Generate(CodeGenerationContext& Context,
	const ptcModule *Module) const
{
	X86Module::Generate(Context, Module);
	if (Context.Result != GR_Success)
		return;
}
