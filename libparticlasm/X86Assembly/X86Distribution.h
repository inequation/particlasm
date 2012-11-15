/*
Particlasm x86 assembly generator distribution interface
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef X86DISTRIBUTION_H
#define X86DISTRIBUTION_H

#include "../libparticlasm.h"
#include "../CodeGeneratorInterface.h"

class X86Distribution
{
	protected:
		// c-tor protected on purpose so that objects of the base class may not
		// be freely created
		X86Distribution(ptcDistributionID InID) : ID(InID) {}

	public:
		inline ptcDistributionID GetID() const { return ID; }

		virtual void Generate(CodeGenerationContext& Context,
			const ptcScalarDistr *Scalar) const
		{
			Context.Result = Scalar->DistrID == GetID()
				? GR_Success
				: GR_DistributionIDMismatch;
		}

		virtual void Generate(CodeGenerationContext& Context,
			const ptcVectorDistr *Vector) const
		{
			Context.Result = Vector->DistrID == GetID()
				? GR_Success
				: GR_DistributionIDMismatch;
		}

		virtual void Generate(CodeGenerationContext& Context,
			const ptcColourDistr *Colour) const
		{
			Context.Result = Colour->DistrID == GetID()
				? GR_Success
				: GR_DistributionIDMismatch;
		}

	private:
		ptcDistributionID ID;
};

#endif // X86DISTRIBUTION_H
