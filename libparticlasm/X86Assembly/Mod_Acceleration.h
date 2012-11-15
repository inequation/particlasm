/*
Particlasm x86 assembly generator module for acceleration
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef MOD_ACCELERATION_H
#define MOD_ACCELERATION_H

#include "X86Module.h"

class Mod_Acceleration : public X86Module
{
	public:
		Mod_Acceleration();

		void Generate(CodeGenerationContext &Context, const ptcModule *Module)
			const;
};

#endif // MOD_ACCELERATION_H
