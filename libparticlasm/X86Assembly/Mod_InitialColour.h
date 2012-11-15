/*
Particlasm x86 assembly generator module for initial colour
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef MOD_INITIALCOLOUR_H
#define MOD_INITIALCOLOUR_H

#include "X86Module.h"

class Mod_InitialColour : public X86Module
{
	public:
		Mod_InitialColour();

		void Generate(CodeGenerationContext &Context, const ptcModule *Module)
			const;
};

#endif // MOD_INITIALCOLOUR_H
