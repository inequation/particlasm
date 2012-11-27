/*
Particlasm main private include file
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef X86ASSEMBLY_H
#define X86ASSEMBLY_H

#include <libparticlasm2.h>
#include <map>

struct PrivateContextData
{
	typedef std::map<const void *, size_t> DataMap;

	DataMap	Map;
	size_t	CurrentDataIndex;

	PrivateContextData() : CurrentDataIndex(0) {}
	inline void Register(const void *Ptr)
	{
		Map.insert(DataMap::value_type(Ptr, CurrentDataIndex++));
	}
	inline size_t Find(const void *Ptr)
	{
		return Map[Ptr];
	}
};

#endif // X86ASSEMBLY_H
