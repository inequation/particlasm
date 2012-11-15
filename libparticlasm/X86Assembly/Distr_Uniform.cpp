/*
Particlasm x86 assembly generator constant distribution
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Distr_Uniform.h"
#include "AsmSnippets.h"

Distr_Uniform::Distr_Uniform() :
	X86Distribution(ptcDID_Uniform)
{
	//ctor
}

void Distr_Uniform::Generate(CodeGenerationContext& Context,
	const ptcScalarDistr *Scalar) const
{
	X86Distribution::Generate(Context, Scalar);
	if (Context.Result != GR_Success)
		return;
	switch (Context.Stage)
	{
		case GS_Data:
			{
				// write as integer to avoid precision loss
				const uint32_t *AsUint = (const uint32_t *)
					&Scalar->Uniform.Range[0];
				Context.Emitf(Asm_SDistr_Uniform_Data,
					Context.CurrentDataIndex++,
					AsUint[0],
					AsUint[1]);
				break;
			}
		case GS_SpawnCode:
		case GS_ProcessCode:
			Context.Emitf(Asm_SDistr_Uniform_Code,
				Context.CurrentDataIndex, Context.CurrentDataIndex);
			++Context.CurrentDataIndex;
			break;
		default:
			assert(!"Invalid stage");
	}
}

void Distr_Uniform::Generate(CodeGenerationContext& Context,
	const ptcVectorDistr *Vector) const
{
	X86Distribution::Generate(Context, Vector);
	if (Context.Result != GR_Success)
		return;
	switch (Context.Stage)
	{
		case GS_Data:
			{
				// write as integer to avoid precision loss
				const uint32_t *AsUint1 = (const uint32_t *)
					&(Vector->Uniform.Ranges[0])[0];
				const uint32_t *AsUint2 = (const uint32_t *)
					&(Vector->Uniform.Ranges[1])[0];
				Context.Emitf(Asm_VDistr_Uniform_Data,
					Context.CurrentDataIndex++,
					AsUint1[0],
					AsUint1[1],
					AsUint1[2],
					AsUint2[0],
					AsUint2[1],
					AsUint2[2]);
				break;
			}
		case GS_SpawnCode:
		case GS_ProcessCode:
			Context.Emitf(Asm_VDistr_Uniform_Code,
				Context.CurrentDataIndex, Context.CurrentDataIndex);
			++Context.CurrentDataIndex;
			break;
		default:
			assert(!"Invalid stage");
	}
}

void Distr_Uniform::Generate(CodeGenerationContext& Context,
	const ptcColourDistr *Colour) const
{
	X86Distribution::Generate(Context, Colour);
	if (Context.Result != GR_Success)
		return;
	switch (Context.Stage)
	{
		case GS_Data:
			{
				// write as integer to avoid precision loss
				const uint32_t *AsUint1 = (const uint32_t *)
					&(Colour->Uniform.Ranges[0])[0];
				const uint32_t *AsUint2 = (const uint32_t *)
					&(Colour->Uniform.Ranges[1])[0];
				Context.Emitf(Asm_CDistr_Uniform_Data,
					Context.CurrentDataIndex++,
					AsUint1[0],
					AsUint1[1],
					AsUint1[2],
					AsUint1[3],
					AsUint2[0],
					AsUint2[1],
					AsUint2[2],
					AsUint2[3]);
				break;
			}
		case GS_SpawnCode:
		case GS_ProcessCode:
			Context.Emitf(Asm_CDistr_Uniform_Code,
				Context.CurrentDataIndex, Context.CurrentDataIndex);
			++Context.CurrentDataIndex;
			break;
		default:
			assert(!"Invalid stage");
	}
}
