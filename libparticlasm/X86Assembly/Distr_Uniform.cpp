/*
Particlasm x86 assembly generator constant distribution
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Distr_Uniform.h"
#include "AsmSnippets/AsmSnippets.h"

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
			Context.Emitf(".Offset%d:", Context.CurrentDataIndex++);
			for (int i = 0; i < 2; ++i)
			{
				// write as integer to avoid precision loss
				const uint32_t *AsUint = (const uint32_t *)
					&Scalar->Uniform.Range[i];
				Context.Emitf("\tfloat_s_reserve 0x%08X\n", *AsUint);
			}
			break;
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
			Context.Emitf(".Offset%d:", Context.CurrentDataIndex++);
			for (int i = 0; i < 2; ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					// write as integer to avoid precision loss
					const uint32_t *AsUint = (const uint32_t *)
						&(Vector->Uniform.Ranges[i])[j];
					Context.Emitf("\tfloat_s_reserve 0x%08X\n", *AsUint);
				}
				// also a dummy value to round off the m128
				Context.Emitf("\tfloat_s_reserve 0x00000000\n");
			}
			break;
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
			Context.Emitf(".Offset%d:", Context.CurrentDataIndex++);
			for (int i = 0; i < 2; ++i)
			{
				for (int j = 0; j < 4; ++j)
				{
					// write as integer to avoid precision loss
					const uint32_t *AsUint = (const uint32_t *)
						&(Colour->Uniform.Ranges[i])[j];
					Context.Emitf("\tfloat_s_reserve 0x%08X\n", *AsUint);
				}
			}
			break;
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
