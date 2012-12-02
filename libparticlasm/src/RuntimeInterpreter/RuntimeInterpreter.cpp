/*
Particlasm runtime interpreter (a.k.a. reference C++ implementation)
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "../Core/ParticlasmMain.h"
#include "RuntimeInterpreter.h"

extern uint32_t ref_ptcCompileEmitter(ptcEmitter *emitter);
extern size_t ref_ptcProcessEmitter(ptcEmitter *emitter,
	float step, ptcVector cameraCS[3], ptcVertex *buffer, size_t maxVertices);
extern void ref_ptcReleaseEmitter(ptcEmitter *emitter);
extern void ref_ptcSpawnParticles(ptcEmitter *emitter, float step,
	uint32_t count);
extern uint32_t ref_ptcProcessParticles(ptcEmitter *emitter,
	ptcParticle *startPtr, ptcParticle *endPtr, float step,
	ptcVector cameraCS[3], ptcVertex *buffer, size_t maxVertices);

// get rid of warnings
SUPPRESS_WARNING_GCC_BEGIN("-Wunused-parameter")

void RuntimeInterpreter::Generate(CodeGenerationContext& Context) const
{
	ref_ptcCompileEmitter(Context.Emitter);
	Context.Result = GR_Success;
}

void RuntimeInterpreter::Build(ConstructionContext& Context,
	char *OutBinaryPath, size_t OutBinaryPathSize) const
{
	Context.Result = CR_Success;
}

bool RuntimeInterpreter::LoadCachedBinary(ptcEmitter *Emitter,
	const void *Buffer, const size_t Size) const
{
	return true;
}

size_t RuntimeInterpreter::WriteCachedBinary(ptcEmitter *Emitter,
	PFNWRITE WriteCallback) const
{
	return 0;
}

bool RuntimeInterpreter::LoadRawBinary(ptcEmitter *Emitter, const void *Buffer,
	const size_t Size, const size_t DataOffset,
	const size_t SpawnCodeOffset, const size_t ProcessCodeOffset) const
{
	return true;
}

void RuntimeInterpreter::Unload(ptcEmitter *Emitter) const
{
	ref_ptcReleaseEmitter(Emitter);
}

void RuntimeInterpreter::SpawnParticles(ptcEmitter *Emitter, float TimeStep,
	uint32_t Count) const
{
	ref_ptcSpawnParticles(Emitter, TimeStep, Count);
}

uint32_t RuntimeInterpreter::ProcessParticles(ptcEmitter *Emitter,
	ptcParticle *StartPtr, ptcParticle *EndPtr, float TimeStep,
	ptcVector CameraCS[3], ptcVertex *Buffer, uint32_t MaxVertices)
	const
{
	return ref_ptcProcessParticles(Emitter, StartPtr, EndPtr, TimeStep,
		CameraCS, Buffer, MaxVertices);
}

SUPPRESS_WARNING_GCC_END
