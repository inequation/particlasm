/*
Particlasm x86 assembly generator constant distribution
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef DISTR_UNIFORM_H
#define DISTR_UNIFORM_H

#include "X86DistributionInterface.h"

class Distr_Uniform : public X86DistributionInterface
{
	public:
		Distr_Uniform();

		void Generate(CodeGenerationContext& Context,
			const ptcScalarDistr *Scalar) const;
		void Generate(CodeGenerationContext& Context,
			const ptcVectorDistr *VectorPtr) const;
		void Generate(CodeGenerationContext& Context,
			const ptcColourDistr *ColourPtr) const;
};

#endif // DISTR_UNIFORM_H
