/*
Particlasm x86 assembly generator module for velocity
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef MOD_VELOCITY_H
#define MOD_VELOCITY_H

#include "X86Module.h"

class Mod_Velocity : public X86Module
{
	public:
		Mod_Velocity();
		virtual ~Mod_Velocity();

		void Generate(CodeGenerationContext &Context, const ptcModule *Module)
			const;
};

#endif // MOD_VELOCITY_H
