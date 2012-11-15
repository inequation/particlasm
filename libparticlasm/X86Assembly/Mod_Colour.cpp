/*
Particlasm x86 assembly generator module for colour
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Mod_Colour.h"
#include "AsmSnippets.h"

Mod_Colour::Mod_Colour() :
	X86Module(ptcMID_Colour)
{
	//ctor
}

void Mod_Colour::Generate(CodeGenerationContext& Context,
	const ptcModule *Module) const
{
	X86Module::Generate(Context, Module);
	if (Context.Result != GR_Success)
		return;
	switch (Context.Stage)
	{
		case GS_Data:
			GenerateDistribution<ptcColourDistr>(Context,
				&Module->Col.Distr);
			if (Context.Result != GR_Success)
				return;
			Context.Emitf(Asm_Mod_Colour_Data,
				Context.CurrentDataIndex++,
				Module->Col.Flags);
			break;
		case GS_ProcessCode:
			Context.Emitf(Asm_Mod_Colour_PreDistr, Context.CurrentDataIndex++);
			GenerateDistribution<ptcColourDistr>(Context,
				&Module->Col.Distr);
			if (Context.Result != GR_Success)
				return;
			Context.Emitf(Asm_Mod_Colour_PostDistr);
			break;
		default:
			// nothing to do
			break;
	}
}
