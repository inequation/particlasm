/*
Particlasm x86 assembly generator module for initial size
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Mod_InitialSize.h"
#include "AsmSnippets.h"

Mod_InitialSize::Mod_InitialSize() :
	X86Module(ptcMID_InitialSize)
{
	//ctor
}

void Mod_InitialSize::Generate(CodeGenerationContext& Context,
	const ptcModule *Module) const
{
	X86Module::Generate(Context, Module);
	if (Context.Result != GR_Success)
		return;
	switch (Context.Stage)
	{
		case GS_Data:
			GenerateDistribution<ptcScalarDistr>(Context,
				&Module->InitSize.Distr);
			if (Context.Result != GR_Success)
				return;
			break;
		case GS_SpawnCode:
			GenerateDistribution<ptcScalarDistr>(Context,
				&Module->InitSize.Distr);
			if (Context.Result != GR_Success)
				return;
			Context.Emitf(Asm_Mod_InitialSize);
			break;
		default:
			// nothing to do
			break;
	}
}
