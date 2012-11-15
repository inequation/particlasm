/*
Particlasm x86 assembly generator constant distribution
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "Distr_Constant.h"
#include "AsmSnippets.h"

Distr_Constant::Distr_Constant() :
	X86Distribution(ptcDID_Constant)
{
	//ctor
}

void Distr_Constant::Generate(CodeGenerationContext& Context,
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
					&Scalar->Constant.Val;
				Context.Emitf(Asm_SDistr_Constant_Data,
					Context.CurrentDataIndex++,
					*AsUint);
				break;
			}
		case GS_SpawnCode:
		case GS_ProcessCode:
			Context.Emitf(Asm_SDistr_Constant_Code,
				Context.CurrentDataIndex++);
			break;
		default:
			assert(!"Invalid stage");
	}
}

void Distr_Constant::Generate(CodeGenerationContext& Context,
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
				const uint32_t *AsUint = (const uint32_t *)
					&Vector->Constant.Val[0];
				Context.Emitf(Asm_CDistr_Constant_Data,
					Context.CurrentDataIndex++,
					AsUint[0],
					AsUint[1],
					AsUint[2]);
				break;
			}
		case GS_SpawnCode:
		case GS_ProcessCode:
			Context.Emitf(Asm_VDistr_Constant_Code,
				Context.CurrentDataIndex++);
			break;
		default:
			assert(!"Invalid stage");
	}
}

void Distr_Constant::Generate(CodeGenerationContext& Context,
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
				const uint32_t *AsUint = (const uint32_t *)
					&Colour->Constant.Val[0];
				Context.Emitf(Asm_CDistr_Constant_Data,
					Context.CurrentDataIndex++,
					AsUint[0],
					AsUint[1],
					AsUint[2],
					AsUint[3]);
				break;
			}
		case GS_SpawnCode:
		case GS_ProcessCode:
			Context.Emitf(Asm_CDistr_Constant_Code,
				Context.CurrentDataIndex++);
			break;
		default:
			assert(!"Invalid stage");
	}
}
