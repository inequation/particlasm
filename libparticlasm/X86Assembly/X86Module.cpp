/*
Particlasm x86 assembly generator module interface
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "X86Module.h"
#include "X86Distribution.h"
#include "Distr_Constant.h"
#include "Distr_Uniform.h"

const X86Distribution* X86Module::DistrMap[] =
{
	new Distr_Constant(),
	new Distr_Uniform()
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
	Context.Result = Module->Header.ModuleID == ID
		? GR_Success
		: GR_ModuleIDMismatch;
}
