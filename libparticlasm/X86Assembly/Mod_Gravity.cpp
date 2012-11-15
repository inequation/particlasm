/*
Particlasm x86 assembly generator module for gravity
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Mod_Gravity.h"

Mod_Gravity::Mod_Gravity() :
	X86Module(ptcMID_Gravity)
{
	//ctor
}

Mod_Gravity::~Mod_Gravity()
{
	//dtor
}

void Mod_Gravity::Generate(CodeGenerationContext& Context,
	const ptcModule *Module) const
{
	X86Module::Generate(Context, Module);
	if (Context.Result != GR_Success)
		return;
}
