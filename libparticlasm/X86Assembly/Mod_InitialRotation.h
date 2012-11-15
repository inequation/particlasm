/*
Particlasm x86 assembly generator module for initial rotation
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef MOD_INITIALROTATION_H
#define MOD_INITIALROTATION_H

#include "X86Module.h"

class Mod_InitialRotation : public X86Module
{
	public:
		Mod_InitialRotation();
		virtual ~Mod_InitialRotation();

		void Generate(CodeGenerationContext &Context, const ptcModule *Module)
			const;
};

#endif // MOD_INITIALROTATION_H
