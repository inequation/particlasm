/*
Particlasm compiled code launcher interface
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef LAUNCHERINTERFACE_H
#define LAUNCHERINTERFACE_H

#include <libparticlasm2.h>
#include <cassert>

typedef size_t (* PFNWRITE)(void *Source, size_t Bytes);

/**
Compiled binary launcher interface. The Curiously Recurring Template Pattern is
used to achieve compile-time polymorphism to avoid unnecessary dereferences at
runtime.
*/
template <class LauncherImplementation>
class LauncherInterface
{
	public:
		static bool LoadCachedBinary(ptcEmitter *Emitter, void *Buffer,
			const size_t Size)
		{
			return LauncherImplementation::LoadCachedBinaryImpl(Emitter,
				Buffer, Size);
		}

		static size_t WriteCachedBinary(ptcEmitter *Emitter,
			PFNWRITE WriteCallback)
		{
			return LauncherImplementation::WriteCachedBinary(Emitter,
				WriteCallback);
		}

		static void LoadRawBinary(ptcEmitter *Emitter, void *Buffer,
			const size_t Size, const size_t DataOffset,
			const size_t SpawnCodeOffset,const size_t ProcessCodeOffset)
		{
			LauncherImplementation::LoadRawBinaryImpl(Emitter, Buffer, Size,
				SpawnCodeOffset, ProcessCodeOffset);
		}

		static size_t Launch(ptcEmitter *Emitter, float Step,
			ptcVector CameraCS[3], ptcVertex *Buffer, size_t MaxVertices)
		{
			return LauncherImplementation::LaunchImpl(Emitter, Step, CameraCS,
				Buffer, MaxVertices);
		}
};

#endif // LAUNCHERINTERFACE_H
