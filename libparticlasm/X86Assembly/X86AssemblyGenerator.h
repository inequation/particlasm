/*
Particlasm x86 assembly generator
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef X86ASSEMBLYGENERATOR_H
#define X86ASSEMBLYGENERATOR_H

#include "../CodeGeneratorInterface.h"
#include "X86Module.h"
#include "Mod_SimulatePre.h"
#include "Mod_SimulatePost.h"
#include "Mod_InitialLocation.h"
#include "Mod_InitialRotation.h"
#include "Mod_InitialSize.h"
#include "Mod_InitialVelocity.h"
#include "Mod_InitialColour.h"
#include "Mod_Velocity.h"
#include "Mod_Acceleration.h"
#include "Mod_Colour.h"
#include "Mod_Size.h"
#include "Mod_Gravity.h"

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
		static const X86Module *ModuleMap[];
};

#endif // X86ASSEMBLYGENERATOR_H
