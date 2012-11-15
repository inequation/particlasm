/*
Particlasm x86 assembly generator module for initial velocity
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Mod_InitialVelocity.h"

Mod_InitialVelocity::Mod_InitialVelocity() :
	X86Module(ptcMID_InitialVelocity)
{
	//ctor
}

Mod_InitialVelocity::~Mod_InitialVelocity()
{
	//dtor
}

void Mod_InitialVelocity::Generate(CodeGenerationContext& Context,
	const ptcModule *Module) const
{
	X86Module::Generate(Context, Module);
	if (Context.Result != GR_Success)
		return;
	switch (Context.Stage)
	{
		case GS_SpawnCode:
			GenerateDistribution<ptcVectorDistr>(Context, &Module->Accel.Distr);
			if (Context.Result != GR_Success)
				return;
			Context.Emitf("\tmovaps\txmm3, xmm5\n");
			break;
		default:
			// nothing to do
			break;
	}
}
