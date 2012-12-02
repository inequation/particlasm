/*
Particlasm runtime interpreter (a.k.a. reference C++ implementation)
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef RUNTIMEINTERPRETER_H
#define RUNTIMEINTERPRETER_H

#include "../Core/CodeGeneratorInterface.h"
#include "../Core/LauncherInterface.h"

class RuntimeInterpreter : public CodeGeneratorInterface, public LauncherInterface
{
	public:
		virtual void Generate(CodeGenerationContext& Context) const;
		virtual void Build(ConstructionContext& Context, char *OutBinaryPath,
			size_t OutBinaryPathSize) const;
		virtual bool LoadCachedBinary(ptcEmitter *Emitter, const void *Buffer,
			const size_t Size) const;

		virtual size_t WriteCachedBinary(ptcEmitter *Emitter,
			PFNWRITE WriteCallback) const;

		virtual bool LoadRawBinary(ptcEmitter *Emitter, const void *Buffer,
			const size_t Size, const size_t DataOffset,
			const size_t SpawnCodeOffset, const size_t ProcessCodeOffset)
			const;

		virtual void Unload(ptcEmitter *Emitter) const;

		virtual void SpawnParticles(ptcEmitter *Emitter, float TimeStep,
			uint32_t Count) const;

		virtual uint32_t ProcessParticles(ptcEmitter *Emitter,
			ptcParticle *StartPtr, ptcParticle *EndPtr, float TimeStep,
			ptcVector CameraCS[3], ptcVertex *Buffer, uint32_t MaxVertices)
			const;
};

#endif // RUNTIMEINTERPRETER_H
