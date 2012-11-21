/*
Particlasm x86 assembly generator module for gravity
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef MOD_GRAVITY_H
#define MOD_GRAVITY_H

#include "X86ModuleInterface.h"

class Mod_Gravity : public X86ModuleInterface
{
	public:
		Mod_Gravity();

		void Generate(CodeGenerationContext &Context, const ptcModule *Module)
			const;
};

#endif // MOD_GRAVITY_H
