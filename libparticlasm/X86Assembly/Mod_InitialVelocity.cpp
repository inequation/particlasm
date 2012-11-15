/*
Particlasm x86 assembly generator module for initial velocity
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Mod_InitialVelocity.h"
#include "AsmSnippets.h"

Mod_InitialVelocity::Mod_InitialVelocity() :
	X86Module(ptcMID_InitialVelocity)
{
	//ctor
}

void Mod_InitialVelocity::Generate(CodeGenerationContext& Context,
	const ptcModule *Module) const
{
	X86Module::Generate(Context, Module);
	if (Context.Result != GR_Success)
		return;
	switch (Context.Stage)
	{
		case GS_Data:
			GenerateDistribution<ptcVectorDistr>(Context,
				&Module->InitVel.Distr);
			if (Context.Result != GR_Success)
				return;
			break;
		case GS_SpawnCode:
			GenerateDistribution<ptcVectorDistr>(Context,
				&Module->InitVel.Distr);
			if (Context.Result != GR_Success)
				return;
			Context.Emitf(Asm_Mod_InitialVelocity);
			break;
		default:
			// nothing to do
			break;
	}
}
