/*
Particlasm x86 assembly generator module for velocity
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Mod_Velocity.h"
#include "AsmSnippets.h"

Mod_Velocity::Mod_Velocity() :
	X86Module(ptcMID_Velocity)
{
	//ctor
}

void Mod_Velocity::Generate(CodeGenerationContext& Context,
	const ptcModule *Module) const
{
	X86Module::Generate(Context, Module);
	if (Context.Result != GR_Success)
		return;
	switch (Context.Stage)
	{
		case GS_Data:
			GenerateDistribution<ptcVectorDistr>(Context,
				&Module->Vel.Distr);
			if (Context.Result != GR_Success)
				return;
			break;
		case GS_ProcessCode:
			GenerateDistribution<ptcVectorDistr>(Context,
				&Module->Vel.Distr);
			if (Context.Result != GR_Success)
				return;
			Context.Emitf(Asm_Mod_Velocity);
			break;
		default:
			// nothing to do
			break;
	}
}
