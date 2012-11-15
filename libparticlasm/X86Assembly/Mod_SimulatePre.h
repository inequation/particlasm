/*
Particlasm x86 assembly generator module for simulation preprocessing
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef MOD_SIMULATEPRE_H
#define MOD_SIMULATEPRE_H

#include "X86Module.h"

class Mod_SimulatePre : public X86Module
{
	public:
		Mod_SimulatePre();

		void Generate(CodeGenerationContext &Context, const ptcModule *Module)
			const;
};

#endif // MOD_SIMULATEPRE_H
