/*
Particlasm x86 assembly generator module for colour
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef MOD_COLOUR_H
#define MOD_COLOUR_H

#include "X86ModuleInterface.h"

class Mod_Colour : public X86ModuleInterface
{
	public:
		Mod_Colour();

		void Generate(CodeGenerationContext &Context, const ptcModule *Module)
			const;
};

#endif // MOD_COLOUR_H
