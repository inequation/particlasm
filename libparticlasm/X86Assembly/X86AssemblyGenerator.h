/*
Particlasm x86 assembly generator
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef X86ASSEMBLYGENERATOR_H
#define X86ASSEMBLYGENERATOR_H

#include "../CodeGeneratorInterface.h"
#include "AsmSnippets/AsmSnippets.h"

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

		X86AssemblyGenerator(EArchitecture InArch, EPlatform InPlatform);
		virtual ~X86AssemblyGenerator();

		virtual void Generate(CodeGenerationContext& Context);

	private:
		EArchitecture	Arch;
		EPlatform		Platform;
};

#endif // X86ASSEMBLYGENERATOR_H
