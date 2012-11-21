/*
Particlasm x86 assembly generator simple module
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef X86SIMPLEMODULE_H
#define X86SIMPLEMODULE_H

#include "X86ModuleInterface.h"

template <typename T>
class X86SimpleModule : public X86ModuleInterface
{
	public:
		X86SimpleModule(ptcModuleID InID,
			const char *InSpawnPostDistrSnippet,
			const char *InProcessPostDistrSnippet,
			const ptrdiff_t InDistrOffset = -1,
			const char *InSpawnPreDistrSnippet = NULL,
			const char *InProcessPreDistrSnippet = NULL)
			:
			X86ModuleInterface(InID),
			SpawnPreDistrSnippet(InSpawnPreDistrSnippet),
			SpawnPostDistrSnippet(InSpawnPostDistrSnippet),
			ProcessPreDistrSnippet(InProcessPreDistrSnippet),
			ProcessPostDistrSnippet(InProcessPostDistrSnippet),
			DistrOffset(InDistrOffset)
			{}

		void Generate(CodeGenerationContext &Context, const ptcModule *Module)
			const
		{
			X86ModuleInterface::Generate(Context, Module);
			if (Context.Result != GR_Success)
				return;
			switch (Context.Stage)
			{
				case GS_Data:
					if (DistrOffset >= 0)
					{
						GenerateDistribution<T>(Context,
							(T *)(((char *)Module) + DistrOffset));
						if (Context.Result != GR_Success)
							return;
					}
					break;
				case GS_SpawnCode:
					if (SpawnPreDistrSnippet)
						Context.Emitf(SpawnPreDistrSnippet);
					if (SpawnPostDistrSnippet)
					{
						if (DistrOffset >= 0)
						{
							GenerateDistribution<T>(Context,
								(T *)(((char *)Module) + DistrOffset));
							if (Context.Result != GR_Success)
								return;
						}
						Context.Emitf(SpawnPostDistrSnippet);
					}
					break;
				case GS_ProcessCode:
					if (ProcessPreDistrSnippet)
						Context.Emitf(ProcessPreDistrSnippet);
					if (ProcessPostDistrSnippet)
					{
						if (DistrOffset >= 0)
						{
							GenerateDistribution<T>(Context,
								(T *)(((char *)Module) + DistrOffset));
							if (Context.Result != GR_Success)
								return;
						}
						Context.Emitf(ProcessPostDistrSnippet);
					}
					break;
				default:
					// nothing to do
					break;
			}
		}

	private:
		const char *SpawnPreDistrSnippet;
		const char *SpawnPostDistrSnippet;
		const char *ProcessPreDistrSnippet;
		const char *ProcessPostDistrSnippet;
		const ptrdiff_t DistrOffset;
};

#endif // X86SIMPLEMODULE_H
