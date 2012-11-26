/*
Particlasm compiled code launcher for X86 assembly
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef X86LAUNCHER_H
#define X86LAUNCHER_H

#include "../Core/LauncherInterface.h"

class X86Launcher : public LauncherInterface
{
	public:
		X86Launcher();

		virtual bool LoadCachedBinary(ptcEmitter *Emitter, const void *Buffer,
			const size_t Size) const;

		virtual size_t WriteCachedBinary(ptcEmitter *Emitter,
			PFNWRITE WriteCallback) const;

		virtual bool LoadRawBinary(ptcEmitter *Emitter, const void *Buffer,
			const size_t Size, const size_t DataOffset,
			const size_t SpawnCodeOffset,const size_t ProcessCodeOffset) const;

		virtual void Unload(ptcEmitter *Emitter) const;

		virtual size_t Launch(ptcEmitter *Emitter, float Step,
			ptcVector CameraCS[3], ptcVertex *Buffer, size_t MaxVertices) const;
};

#endif // X86LAUNCHER_H
