/*
Particlasm x86 assembly generator module interface
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef X86MODULE_H
#define X86MODULE_H

#include "../libparticlasm.h"
#include "../CodeGeneratorInterface.h"
#include "X86DistributionInterface.h"

class X86ModuleInterface
{
	protected:
		// c-tor protected on purpose so that objects of the base class may not
		// be freely created
		X86ModuleInterface(ptcModuleID InID);

	public:
		inline ptcModuleID GetID() const { return ID; }

		virtual void Generate(CodeGenerationContext &Context,
			const ptcModule *Module) const;

	protected:
		template <typename T>
		void GenerateDistribution(CodeGenerationContext& Context,
			const T *Distr) const
		{
			size_t i;
			for (i = 0; i < DistrMapCount; ++i)
			{
				DistrMap[i]->Generate(Context, Distr);
				if (Context.Result == GR_Success)
					// break the loop upon success
					break;
				else if (Context.Result == GR_DistributionIDMismatch)
					// try another distribution
					continue;
				else
					return;
			}

			if (i >= DistrMapCount)
			{
				Context.Result = GR_UnsupportedDistributionID;
				Context.ResultArgument = Distr->DistrID;
			}
		}

	private:
		ptcModuleID ID;

		static const X86DistributionInterface *DistrMap[];
		static const size_t DistrMapCount;
};

#endif // X86MODULE_H
