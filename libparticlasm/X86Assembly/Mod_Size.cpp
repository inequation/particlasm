/*
Particlasm x86 assembly generator module for size
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Mod_Size.h"

Mod_Size::Mod_Size() :
	X86Module(ptcMID_Size)
{
	//ctor
}

Mod_Size::~Mod_Size()
{
	//dtor
}

void Mod_Size::Generate(CodeGenerationContext& Context,
	const ptcModule *Module) const
{
	X86Module::Generate(Context, Module);
	if (Context.Result != GR_Success)
		return;
}
