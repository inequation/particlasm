/*
Particlasm x86 assembly generator module for initial size
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Mod_InitialSize.h"

Mod_InitialSize::Mod_InitialSize() :
	X86Module(ptcMID_InitialSize)
{
	//ctor
}

Mod_InitialSize::~Mod_InitialSize()
{
	//dtor
}

void Mod_InitialSize::Generate(CodeGenerationContext& Context,
	const ptcModule *Module) const
{
	X86Module::Generate(Context, Module);
	if (Context.Result != GR_Success)
		return;
}
