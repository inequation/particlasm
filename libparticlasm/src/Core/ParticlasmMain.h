/*
Particlasm main private include file
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#ifndef PARTICLASMMAIN_H
#define PARTICLASMMAIN_H

#include <libparticlasm2.h>
#include "../Config.h"

// choose an appropriate symbol export declaration
#if defined(WIN32) || defined(__WIN32__)
	#define EXPORTDECL	__declspec(dllexport)
#elif defined(__BEOS__) && !defined(__GNUC__)
	#define EXPORTDECL	__declspec(export)
#elif defined(__GNUC__) && __GNUC__ >= 4
	#define EXPORTDECL	__attribute__ ((visibility("default")))
#else
	#define EXPORTDECL
	#warning EXPORTDECL is empty for this compiler
#endif

// warning suppression macros
#define STRINGIFY(x)	#x
#if defined(__GNUC__)
	#define SUPPRESS_WARNING_GCC_BEGIN(id)				\
		_Pragma("GCC diagnostic push")					\
		_Pragma(STRINGIFY(GCC diagnostic ignored id))
	#define SUPPRESS_WARNING_GCC_END					\
		_Pragma("GCC diagnostic pop")
	#define SUPPRESS_WARNING_MSVC_BEGIN
	#define SUPPRESS_WARNING_MSVC_END
#elif defined(_MSC_VER)
	#define SUPPRESS_WARNING_GCC_BEGIN
	#define SUPPRESS_WARNING_GCC_END
	#define SUPPRESS_WARNING_MSVC_BEGIN(id)				\
		__pragma(warning(push))							\
		__pragma(warning(disable : vc_id))
	#define SUPPRESS_WARNING_MSVC_END					\
		__pragma(warning(pop))
#else
	#define SUPPRESS_WARNING_GCC_BEGIN
	#define SUPPRESS_WARNING_GCC_END
	#define SUPPRESS_WARNING_MSVC_BEGIN
	#define SUPPRESS_WARNING_MSVC_END
#endif	// __GNUC__
#define SUPPRESS_WARNING_BEGIN(gcc_id, msvc_id)			\
	SUPPRESS_WARNING_GCC_BEGIN(gcc_id)					\
	SUPPRESS_WARNING_MSVC_BEGIN(msvc_id)
#define SUPPRESS_WARNING_END							\
	SUPPRESS_WARNING_GCC_END							\
	SUPPRESS_WARNING_MSVC_END

#endif // PARTICLASMMAIN_H
