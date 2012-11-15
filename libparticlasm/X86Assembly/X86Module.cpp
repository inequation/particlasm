/*
Particlasm x86 assembly generator module interface
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "X86Module.h"
#include "X86Distribution.h"
#include "Distr_Constant.h"

const X86Distribution* X86Module::DistrMap[] =
{
	new Distr_Constant()
};
const size_t X86Module::DistrMapCount = sizeof(DistrMap) / sizeof(DistrMap[0]);

X86Module::X86Module(ptcModuleID InID) :
	ID(InID)
{
	//ctor
}

void X86Module::Generate(
	CodeGenerationContext &Context, const ptcModule *Module) const
{
	if (Module->Header.ModuleID != ID)
		Context.Result = GR_ModuleIDMismatch;
}
