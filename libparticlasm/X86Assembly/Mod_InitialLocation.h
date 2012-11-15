/*
Particlasm x86 assembly generator module for initial location
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef MOD_INITIALLOCATION_H
#define MOD_INITIALLOCATION_H

#include "X86Module.h"

class Mod_InitialLocation : public X86Module
{
	public:
		Mod_InitialLocation();

		void Generate(CodeGenerationContext &Context, const ptcModule *Module)
			const;
};

#endif // MOD_INITIALLOCATION_H
