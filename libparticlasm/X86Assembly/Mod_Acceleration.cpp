/*
Particlasm x86 assembly generator module for acceleration
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Mod_Acceleration.h"

Mod_Acceleration::Mod_Acceleration() :
	X86Module(ptcMID_Acceleration)
{
	//ctor
}

Mod_Acceleration::~Mod_Acceleration()
{
	//dtor
}

void Mod_Acceleration::Generate(CodeGenerationContext& Context,
	const ptcModule *Module) const
{
	X86Module::Generate(Context, Module);
	if (Context.Result != GR_Success)
		return;
	switch (Context.Stage)
	{
		case GS_ProcessCode:
			GenerateDistribution<ptcVectorDistr>(Context, &Module->Accel.Distr);
			if (Context.Result != GR_Success)
				return;
			Context.Emitf("\tmovaps\txmm4, xmm5\n");
			break;
		default:
			// nothing to do
			break;
	}
}
