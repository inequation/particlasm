/*
Particlasm x86 assembly generator module for initial size
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef MOD_INITIALSIZE_H
#define MOD_INITIALSIZE_H

#include "X86Module.h"

class Mod_InitialSize : public X86Module
{
	public:
		Mod_InitialSize();
		virtual ~Mod_InitialSize();

		void Generate(CodeGenerationContext &Context, const ptcModule *Module)
			const;
};

#endif // MOD_INITIALSIZE_H
