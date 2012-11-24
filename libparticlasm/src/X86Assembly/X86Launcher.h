/*
Particlasm compiled code launcher for X86 assembly
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef X86LAUNCHER_H
#define X86LAUNCHER_H

#include "../Core/LauncherInterface.h"

class X86Launcher : public LauncherInterface<X86Launcher>
{
	friend class LauncherInterface<X86Launcher>;
	private:
		static bool LoadCachedBinaryImpl(ptcEmitter *Emitter, void *Buffer,
			const size_t Size);

		static size_t WriteCachedBinaryImpl(ptcEmitter *Emitter,
			PFNWRITE WriteCallback);

		static void LoadRawBinaryImpl(ptcEmitter *Emitter, void *Buffer,
			const size_t Size, const size_t DataOffset,
			const size_t SpawnCodeOffset,const size_t ProcessCodeOffset);

		static size_t LaunchImpl(ptcEmitter *Emitter, float Step,
			ptcVector CameraCS[3], ptcVertex *Buffer, size_t MaxVertices);
};

#endif // X86LAUNCHER_H
