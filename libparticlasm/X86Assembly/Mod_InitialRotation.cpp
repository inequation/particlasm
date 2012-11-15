/*
Particlasm x86 assembly generator module for initial rotation
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Mod_InitialRotation.h"
#include "AsmSnippets.h"

Mod_InitialRotation::Mod_InitialRotation() :
	X86Module(ptcMID_InitialRotation)
{
	//ctor
}

void Mod_InitialRotation::Generate(CodeGenerationContext& Context,
	const ptcModule *Module) const
{
	X86Module::Generate(Context, Module);
	if (Context.Result != GR_Success)
		return;
	switch (Context.Stage)
	{
		case GS_Data:
			GenerateDistribution<ptcScalarDistr>(Context,
				&Module->InitRot.Distr);
			if (Context.Result != GR_Success)
				return;
			break;
		case GS_SpawnCode:
			GenerateDistribution<ptcScalarDistr>(Context,
				&Module->InitRot.Distr);
			if (Context.Result != GR_Success)
				return;
			Context.Emitf(Asm_Mod_InitialRotation);
			break;
		default:
			// nothing to do
			break;
	}
}
