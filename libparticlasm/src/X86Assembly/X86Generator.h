/*
Particlasm x86 assembly generator
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef X86ASSEMBLYGENERATOR_H
#define X86ASSEMBLYGENERATOR_H

#include "../Core/CodeGeneratorInterface.h"

// some forward declarations to avoid cluttering with includes
class Mod_SimulatePre;
class Mod_SimulatePost;
class X86ModuleInterface;

class X86Generator : public CodeGeneratorInterface
{
	public:
		typedef enum
		{
			ARCH_x86,
			ARCH_x86_64
		}
		EArchitecture;

		X86Generator(EArchitecture InArch);

		virtual void Generate(CodeGenerationContext& Context) const;
		virtual void Build(ConstructionContext& Context, char *OutBinaryPath,
			size_t OutBinaryPathSize) const;

	private:
		EArchitecture	Arch;
		static const Mod_SimulatePre SimPre;
		static const Mod_SimulatePost SimPost;
		static const X86ModuleInterface *ModuleMap[];

		void FindCodeOffsets(ConstructionContext& Context,
			const char *ListFileName) const;
};

#endif // X86ASSEMBLYGENERATOR_H
