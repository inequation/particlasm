/*
Particlasm x86 assembly generator module for initial location
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef MOD_INITIALVELOCITY_H
#define MOD_INITIALVELOCITY_H

#include "X86Module.h"

class Mod_InitialVelocity : public X86Module
{
	public:
		Mod_InitialVelocity();

		void Generate(CodeGenerationContext &Context, const ptcModule *Module)
			const;
};

#endif // MOD_INITIALVELOCITY_H
