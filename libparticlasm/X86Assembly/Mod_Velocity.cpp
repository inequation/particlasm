/*
Particlasm x86 assembly generator module for velocity
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Mod_Velocity.h"

Mod_Velocity::Mod_Velocity() :
	X86Module(ptcMID_Velocity)
{
	//ctor
}

Mod_Velocity::~Mod_Velocity()
{
	//dtor
}

void Mod_Velocity::Generate(CodeGenerationContext& Context,
	const ptcModule *Module) const
{
	X86Module::Generate(Context, Module);
	if (Context.Result != GR_Success)
		return;
}
