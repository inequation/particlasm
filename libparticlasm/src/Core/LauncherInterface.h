/*
Particlasm compiled code launcher interface
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef LAUNCHERINTERFACE_H
#define LAUNCHERINTERFACE_H

#include <libparticlasm2.h>
#include <cassert>

typedef size_t (* PFNWRITE)(void *Source, size_t Bytes);

class LauncherInterface
{
	public:
		virtual ~LauncherInterface() {};

		virtual bool LoadCachedBinary(ptcEmitter *Emitter, const void *Buffer,
			const size_t Size) const = 0;

		virtual size_t WriteCachedBinary(ptcEmitter *Emitter,
			PFNWRITE WriteCallback) const = 0;

		virtual bool LoadRawBinary(ptcEmitter *Emitter, const void *Buffer,
			const size_t Size, const size_t DataOffset,
			const size_t SpawnCodeOffset, const size_t ProcessCodeOffset)
			const = 0;

		virtual void Unload(ptcEmitter *Emitter) const = 0;

		virtual void SpawnParticles(ptcEmitter *Emitter, float TimeStep,
			uint32_t Count) const = 0;

		virtual uint32_t ProcessParticles(ptcEmitter *Emitter,
			ptcParticle *StartPtr, ptcParticle *EndPtr, float TimeStep,
			ptcVector CameraCS[3], ptcVertex *Buffer, uint32_t MaxVertices)
			const = 0;
};

#endif // LAUNCHERINTERFACE_H
