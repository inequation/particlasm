/*
Particlasm x86 assembly generator module for initial colour
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Mod_InitialColour.h"
#include "AsmSnippets.h"

Mod_InitialColour::Mod_InitialColour() :
	X86Module(ptcMID_InitialColour)
{
	//ctor
}

void Mod_InitialColour::Generate(CodeGenerationContext& Context,
	const ptcModule *Module) const
{
	X86Module::Generate(Context, Module);
	if (Context.Result != GR_Success)
		return;
	switch (Context.Stage)
	{
		case GS_Data:
			GenerateDistribution<ptcColourDistr>(Context,
				&Module->InitCol.Distr);
			if (Context.Result != GR_Success)
				return;
			Context.Emitf(Asm_Mod_InitialColour_Data,
				Context.CurrentDataIndex++,
				/*Module->InitCol.Flags*/ptcCF_SetRGB | ptcCF_SetAlpha);
			break;
		case GS_SpawnCode:
			Context.Emitf(Asm_Mod_InitialColour_PreDistr,
				Context.CurrentDataIndex++);
			GenerateDistribution<ptcColourDistr>(Context,
				&Module->InitCol.Distr);
			if (Context.Result != GR_Success)
				return;
			Context.Emitf(Asm_Mod_InitialColour_PostDistr);
			break;
		default:
			// nothing to do
			break;
	}
}
