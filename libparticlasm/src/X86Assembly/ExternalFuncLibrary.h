/*
Particlasm 2 external functions library header
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>

The structure containing pointers to functions (mostly C standard library)
exposed to assembly modules is declared here.

This file is also used for automatic NASM declaration generation (see the
GenerateAsmInclude.py script).
*/

/// Such a struct is placed on the stack immediately before calling an
/// individual module, and so it may be accessed at [bp + 2 * sizeof(ptr_t)],
/// or, more safely, via the extlib macro.
/// The prototypes should all be void foo(void), and arguments and return values
/// handled by stack with inline assembly on the C++ side.
typedef struct {
	/// \return	a random 32-bit float in the [-1; 1] range in st0
	void (* FRand)(void);
} ptcExtLib;
