/*
Particlasm x86 assembly generator module for simulation postprocessing
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef MOD_SIMULATEPOST_H
#define MOD_SIMULATEPOST_H

#include "X86Module.h"

class Mod_SimulatePost : public X86Module
{
	public:
		Mod_SimulatePost();

		void Generate(CodeGenerationContext &Context, const ptcModule *Module)
			const;
};

#endif // MOD_SIMULATEPOST_H
