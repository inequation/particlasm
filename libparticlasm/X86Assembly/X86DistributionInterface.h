/*
Particlasm x86 assembly generator distribution interface
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef X86DISTRIBUTION_H
#define X86DISTRIBUTION_H

#include "../libparticlasm2.h"
#include "../CodeGeneratorInterface.h"

class X86DistributionInterface
{
	protected:
		// c-tor protected on purpose so that objects of the base class may not
		// be freely created
		X86DistributionInterface(ptcDistributionID InID) : ID(InID) {}

	public:
		inline ptcDistributionID GetID() const { return ID; }

		virtual void Generate(CodeGenerationContext& Context,
			const ptcScalarDistr *Scalar) const
		{
			if (Scalar->DistrID != GetID())
			{
				Context.Result = GR_DistributionIDMismatch;
				Context.ResultArgument = Scalar->DistrID;
			}
			else
			{
				Context.Result = GR_Success;
				Context.ResultArgument = 0;
			}
		}

		virtual void Generate(CodeGenerationContext& Context,
			const ptcVectorDistr *Vector) const
		{
			if (Vector->DistrID != GetID())
			{
				Context.Result = GR_DistributionIDMismatch;
				Context.ResultArgument = Vector->DistrID;
			}
			else
			{
				Context.Result = GR_Success;
				Context.ResultArgument = 0;
			}
		}

		virtual void Generate(CodeGenerationContext& Context,
			const ptcColourDistr *Colour) const
		{
			if (Colour->DistrID != GetID())
			{
				Context.Result = GR_DistributionIDMismatch;
				Context.ResultArgument = Colour->DistrID;
			}
			else
			{
				Context.Result = GR_Success;
				Context.ResultArgument = 0;
			}
		}

	private:
		ptcDistributionID ID;
};

#endif // X86DISTRIBUTION_H
