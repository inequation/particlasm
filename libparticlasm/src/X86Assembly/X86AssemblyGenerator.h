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

class X86AssemblyGenerator : public CodeGeneratorInterface
{
	public:
		typedef enum
		{
			ARCH_x86,
			ARCH_x86_64
		}
		EArchitecture;

		typedef enum
		{
			PLATFORM_Linux,
			PLATFORM_Windows
		}
		EPlatform;

		X86AssemblyGenerator(EArchitecture InArch, EPlatform InPlatform,
			char *CodeFileName, size_t CodeFileNameSize);

		virtual void Generate(CodeGenerationContext& Context) const;
		virtual void Build(ConstructionContext& Context) const;

	private:
		EArchitecture	Arch;
		EPlatform		Platform;

		static const Mod_SimulatePre SimPre;
		static const Mod_SimulatePost SimPost;
		static const X86ModuleInterface *ModuleMap[];

		void FindCodeOffsets(ConstructionContext& Context,
			const char *ListFileName) const;
};

#endif // X86ASSEMBLYGENERATOR_H
