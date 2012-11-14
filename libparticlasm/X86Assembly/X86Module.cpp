/*
Particlasm x86 assembly generator module interface
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "X86Module.h"

X86Module::X86Module(ptcModuleID InID) :
	ID(InID)
{
	//ctor
}

X86Module::~X86Module()
{
	//dtor
}

void X86Module::Generate(
	CodeGenerationContext &Context, const ptcModule *Module) const
{
	if (Module->Header.ModuleID != ID)
		Context.Result = GR_ModuleIDMismatch;
}
