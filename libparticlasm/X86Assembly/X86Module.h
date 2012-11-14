/*
Particlasm x86 assembly generator module interface
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef X86MODULE_H
#define X86MODULE_H

#include "../libparticlasm.h"
#include "../CodeGeneratorInterface.h"

#define STRINGIFY(a)  #a

class X86Module
{
	protected:
		// c-tor protected on purpose so that objects of the base class may not
		// be freely created
		X86Module(ptcModuleID InID);

	public:
		virtual ~X86Module();

		ptcModuleID GetID() const { return ID; }

		virtual void Generate(CodeGenerationContext &Context,
			const ptcModule *Module) const;

	private:
		ptcModuleID ID;
};

#endif // X86MODULE_H
