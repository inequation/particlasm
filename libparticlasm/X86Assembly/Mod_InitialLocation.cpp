/*
Particlasm x86 assembly generator module for initial location
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Mod_InitialLocation.h"
#include "AsmSnippets.h"

Mod_InitialLocation::Mod_InitialLocation() :
	X86Module(ptcMID_InitialLocation)
{
	//ctor
}

void Mod_InitialLocation::Generate(CodeGenerationContext& Context,
	const ptcModule *Module) const
{
	X86Module::Generate(Context, Module);
	if (Context.Result != GR_Success)
		return;
	switch (Context.Stage)
	{
		case GS_Data:
			GenerateDistribution<ptcVectorDistr>(Context,
				&Module->InitLoc.Distr);
			if (Context.Result != GR_Success)
				return;
			break;
		case GS_SpawnCode:
			GenerateDistribution<ptcVectorDistr>(Context,
				&Module->InitLoc.Distr);
			if (Context.Result != GR_Success)
				return;
			Context.Emitf(Asm_Mod_InitialLocation);
			break;
		default:
			// nothing to do
			break;
	}
}
