/*
Particlasm x86 assembly generator module for size
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Mod_Size.h"
#include "AsmSnippets.h"

Mod_Size::Mod_Size() :
	X86Module(ptcMID_Size)
{
	//ctor
}

void Mod_Size::Generate(CodeGenerationContext& Context,
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
			GenerateDistribution<ptcScalarDistr>(Context,
				&Module->Size.Distr);
			if (Context.Result != GR_Success)
				return;
			Context.Emitf(Asm_Mod_Size);
			break;
		default:
			// nothing to do
			break;
	}
}
