# libparticlasm makefile
# Copyright (C) 2011-2012, Leszek Godlewski <lg@ineqution.org>

# =============================================================================
# Configuration
# Feel free to change any of these, as long as you know what you're doing.
# =============================================================================

# architecture: "x86" for 32-bit, "x64" for 64-bit
ARCH = x64
# platform: "windows" or "linux"
PLATFORM = linux
# path to make
MAKE = make
# mkdir command with arguments to create all missing directories in the path
MKDIR_P = mkdir -p
# path to the Python interpreter
PYTHON = python
# path to the nasm executable
AS = nasm
# additional nasm flags
ASFLAGS =
# path to the C compiler
CC = gcc
# additional CC flags
CFLAGS = -fPIC -c -Wall -Wextra -mfpmath=sse
# path to the linker executable
LINKER = ld
# additional linker flags
LINKERFLAGS = -shared -lc
# debug dist directory
DEBUG_DIST_DIR = ../bin/Debug
# release dist directory
RELEASE_DIST_DIR = ../bin/Release

# =============================================================================
# End of configuration
# =============================================================================

# architecture/platform translation to tool flags
# $(ASFLAGS) comes last so that nasm learns about the target platform first
ifeq ($(ARCH), x64)
	ifeq ($(PLATFORM), linux)
		REAL_ASFLAGS = -f elf64 $(ASFLAGS)
		REAL_CFLAGS = $(CFLAGS) -march=core2 -m64
		REAL_LINKERFLAGS = $(LINKERFLAGS) -melf_x86_64
		DEBUG_FORMAT = dwarf
	else
		REAL_ASFLAGS = -f win64 $(ASFLAGS)
		REAL_CFLAGS = $(CFLAGS) -march=core2 -m64
		REAL_LINKERFLAGS = $(LINKERFLAGS) -mi386pe
		DEBUG_FORMAT = null
	endif
else
	ifeq ($(PLATFORM), linux)
		REAL_ASFLAGS = -f elf32 $(ASFLAGS)
		REAL_CFLAGS = $(CFLAGS) -march=pentium-m -m32
		REAL_LINKERFLAGS = $(LINKERFLAGS) -melf_i386
		DEBUG_FORMAT = dwarf
	else
		REAL_ASFLAGS = -f win32 $(ASFLAGS)
		REAL_CFLAGS = $(CFLAGS) -march=pentium-m -m32
		REAL_LINKERFLAGS = $(LINKERFLAGS) -mi386pe
		DEBUG_FORMAT = null
	endif
endif

all: release

clean:
	rm -f *.o *.so

# recursively call make with proper flags
release:
	$(MAKE) clean
	$(MAKE) libparticlasm.so ARCH=$(ARCH) ASFLAGS="$(ASFLAGS) -Ox" CFLAGS="$(CFLAGS) -O3" LINKERFLAGS="$(LINKERFLAGS) -S"
	$(MKDIR_P) $(RELEASE_DIST_DIR)
	mv libparticlasm.so $(RELEASE_DIST_DIR)

# recursively call make with proper flags
debug:
	$(MAKE) clean
	$(MAKE) libparticlasm.so ARCH=$(ARCH) ASFLAGS="$(ASFLAGS) -O0 -g -F $(DEBUG_FORMAT)" CFLAGS="$(CFLAGS) -O0 -g" LINKERFLAGS="$(LINKERFLAGS)"
	$(MKDIR_P) $(DEBUG_DIST_DIR)
	mv libparticlasm.so $(DEBUG_DIST_DIR)

libparticlasm.so: particlasm.o ptc_highlevel.o
	$(LINKER) $(REAL_LINKERFLAGS) particlasm.o ptc_highlevel.o -o libparticlasm.so

particlasm.o: particl.asm ptc_modules.inc ptc_distributions.inc libparticlasm.inc
	echo "Assembling..."
	$(AS) $(REAL_ASFLAGS) particl.asm -o particlasm.o

ptc_highlevel.o: ptc_highlevel.c libparticlasm.h
	echo "Compiling high level code..."
	$(CC) $(REAL_CFLAGS) ptc_highlevel.c -o ptc_highlevel.o

libparticlasm.inc: libparticlasm.h gen_asm_decls.py
	$(PYTHON) gen_asm_decls.py
