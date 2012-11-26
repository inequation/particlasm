/*
Particlasm x86 assembly generator module for colour
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "X86Assembly.h"
#include "Mod_Colour.h"
#include "AsmSnippets.h"

Mod_Colour::Mod_Colour() :
	X86ModuleInterface(ptcMID_Colour)
{
	//ctor
}

void Mod_Colour::Generate(CodeGenerationContext& Context,
	const ptcModule *Module) const
{
	X86ModuleInterface::Generate(Context, Module);
	if (Context.Result != GR_Success)
		return;
	PrivateContextData *PCD = (PrivateContextData *)Context.PrivateData;
	switch (Context.Stage)
	{
		case GS_Data:
			GenerateDistribution<ptcColourDistr>(Context,
				&Module->Col.Distr);
			if (Context.Result != GR_Success)
				return;
			Context.Emitf(Asm_Mod_Colour_Data,
				PCD->CurrentDataIndex, Module->Col.Flags);
			PCD->Register(Module);
			break;
		case GS_ProcessCode:
			Context.Emitf(Asm_Mod_Colour_PreDistr, PCD->Find(Module));
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
