/*
Particlasm x86 assembly generator constant distribution
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef DISTR_CONSTANT_H
#define DISTR_CONSTANT_H

#include "X86DistributionInterface.h"

class Distr_Constant : public X86DistributionInterface
{
	public:
		Distr_Constant();

		void Generate(CodeGenerationContext& Context,
			const ptcScalarDistr *Scalar) const;
		void Generate(CodeGenerationContext& Context,
			const ptcVectorDistr *VectorPtr) const;
		void Generate(CodeGenerationContext& Context,
			const ptcColourDistr *ColourPtr) const;
};

#endif // DISTR_CONSTANT_H
