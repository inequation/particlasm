/*
Particlasm x86 assembly generator constant distribution
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "X86Assembly.h"
#include "Distr_Constant.h"
#include "AsmSnippets.h"

Distr_Constant::Distr_Constant() :
	X86DistributionInterface(ptcDID_Constant)
{
	//ctor
}

void Distr_Constant::Generate(CodeGenerationContext& Context,
	const ptcScalarDistr *Scalar) const
{
	X86DistributionInterface::Generate(Context, Scalar);
	if (Context.Result != GR_Success)
		return;
	PrivateContextData *PCD = (PrivateContextData *)Context.PrivateData;
	switch (Context.Stage)
	{
		case GS_Data:
			{
				// write as integer to avoid precision loss
				const uint32_t *AsUint = (const uint32_t *)
					&Scalar->Constant.Val;
				Context.Emitf(Asm_SDistr_Constant_Data,
					PCD->CurrentDataIndex, *AsUint);
				// register a pointer-index mapping
				PCD->Register(Scalar);
				break;
			}
		case GS_SpawnCode:
		case GS_ProcessCode:
			Context.Emitf(Asm_SDistr_Constant_Code,
				PCD->Find(Scalar));
			break;
		default:
			assert(!"Invalid stage");
	}
}

void Distr_Constant::Generate(CodeGenerationContext& Context,
	const ptcVectorDistr *Vector) const
{
	X86DistributionInterface::Generate(Context, Vector);
	if (Context.Result != GR_Success)
		return;
	PrivateContextData *PCD = (PrivateContextData *)Context.PrivateData;
	switch (Context.Stage)
	{
		case GS_Data:
			{
				// write as integer to avoid precision loss
				const uint32_t *AsUint = (const uint32_t *)
					&Vector->Constant.Val[0];
				Context.Emitf(Asm_CDistr_Constant_Data,
					PCD->CurrentDataIndex,
					AsUint[0],
					AsUint[1],
					AsUint[2]);
				PCD->Register(Vector);
				break;
			}
		case GS_SpawnCode:
		case GS_ProcessCode:
			Context.Emitf(Asm_VDistr_Constant_Code,
				PCD->Find(Vector));
			break;
		default:
			assert(!"Invalid stage");
	}
}

void Distr_Constant::Generate(CodeGenerationContext& Context,
	const ptcColourDistr *Colour) const
{
	X86DistributionInterface::Generate(Context, Colour);
	if (Context.Result != GR_Success)
		return;
	PrivateContextData *PCD = (PrivateContextData *)Context.PrivateData;
	switch (Context.Stage)
	{
		case GS_Data:
			{
				// write as integer to avoid precision loss
				const uint32_t *AsUint = (const uint32_t *)
					&Colour->Constant.Val[0];
				Context.Emitf(Asm_CDistr_Constant_Data,
					PCD->CurrentDataIndex,
					AsUint[0],
					AsUint[1],
					AsUint[2],
					AsUint[3]);
				PCD->Register(Colour);
				break;
			}
		case GS_SpawnCode:
		case GS_ProcessCode:
			Context.Emitf(Asm_CDistr_Constant_Code,
				PCD->Find(Colour));
			break;
		default:
			assert(!"Invalid stage");
	}
}
