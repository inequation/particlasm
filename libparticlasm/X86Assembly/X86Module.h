/*
Particlasm x86 assembly generator module interface
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef X86MODULE_H
#define X86MODULE_H

#include "../libparticlasm.h"
#include "../CodeGeneratorInterface.h"
#include "X86Distribution.h"

class X86Module
{
	protected:
		// c-tor protected on purpose so that objects of the base class may not
		// be freely created
		X86Module(ptcModuleID InID);

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
				if (Context.Result == GR_DistributionIDMismatch)
				{
					Context.Result = GR_Success;
					continue;
				}
				else if (Context.Result != GR_Success)
					return;
				// break the loop upon success
				break;
			}

			if (i >= DistrMapCount)
				Context.Result = GR_UnsupportedDistributionID;
		}

	private:
		ptcModuleID ID;

		static const X86Distribution *DistrMap[];
		static const size_t DistrMapCount;
};

#endif // X86MODULE_H
