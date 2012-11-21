/*
Particlasm x86 assembly generator module interface
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "X86ModuleInterface.h"
#include "X86DistributionInterface.h"
#include "Distr_Constant.h"
#include "Distr_Uniform.h"

const X86DistributionInterface* X86ModuleInterface::DistrMap[] =
{
	new Distr_Constant(),
	new Distr_Uniform()
};
const size_t X86ModuleInterface::DistrMapCount = sizeof(DistrMap) / sizeof(DistrMap[0]);

X86ModuleInterface::X86ModuleInterface(ptcModuleID InID) :
	ID(InID)
{
	//ctor
}

void X86ModuleInterface::Generate(
	CodeGenerationContext &Context, const ptcModule *Module) const
{
	if (Module->Header.ModuleID != ID)
	{
		Context.Result = GR_ModuleIDMismatch;
		Context.ResultArgument = Module->Header.ModuleID;
	}
	else
	{
		Context.Result = GR_Success;
		Context.ResultArgument = 0;
	}
}
