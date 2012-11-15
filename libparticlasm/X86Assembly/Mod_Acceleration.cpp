/*
Particlasm x86 assembly generator module for acceleration
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Mod_Acceleration.h"
#include "AsmSnippets.h"

Mod_Acceleration::Mod_Acceleration() :
	X86Module(ptcMID_Acceleration)
{
	//ctor
}

void Mod_Acceleration::Generate(CodeGenerationContext& Context,
	const ptcModule *Module) const
{
	X86Module::Generate(Context, Module);
	if (Context.Result != GR_Success)
		return;
	switch (Context.Stage)
	{
		case GS_Data:
			GenerateDistribution<ptcVectorDistr>(Context,
				&Module->Accel.Distr);
			if (Context.Result != GR_Success)
				return;
			break;
		case GS_ProcessCode:
			GenerateDistribution<ptcVectorDistr>(Context,
				&Module->Accel.Distr);
			if (Context.Result != GR_Success)
				return;
			Context.Emitf(Asm_Mod_Acceleration);
			break;
		default:
			// nothing to do
			break;
	}
}
