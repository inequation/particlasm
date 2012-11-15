/*
Particlasm x86 assembly generator module for size
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef MOD_SIZE_H
#define MOD_SIZE_H

#include "X86Module.h"

class Mod_Size : public X86Module
{
	public:
		Mod_Size();
		virtual ~Mod_Size();

		void Generate(CodeGenerationContext &Context, const ptcModule *Module)
			const;
};

#endif // MOD_SIZE_H
