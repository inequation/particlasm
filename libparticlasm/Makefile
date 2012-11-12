# libparticlasm makefile
# Copyright (C) 2011-2012, Leszek Godlewski <lg@ineqution.org>

# =============================================================================
# Configuration
# Feel free to change any of these, as long as you know what you're doing.
# =============================================================================

# default architecture: "x86" for 32-bit, "x64" for 64-bit
ARCH=x64
# default platform: "windows" or "linux"
PLATFORM=linux
# path to make
MAKE=make
# mkdir command with arguments to create all missing directories in the path
MKDIR_P=mkdir -p
# path to the Python interpreter
PYTHON=python
# path to the nasm executable
AS=nasm
# additional nasm flags
ASFLAGS =
# path to the C compiler
CC=gcc
# additional CC flags
CFLAGS=-fPIC -c -Wall -Wextra -mfpmath=sse
# path to the linker executable
LD=ld
# additional linker flags
LDFLAGS=-shared -lc
# debug dist directory
DEBUG_DIST_DIR=../bin/Debug
# release dist directory
RELEASE_DIST_DIR=../bin/Release
# output library file name
OUTPUT_NAME=libparticlasm

# =============================================================================
# End of configuration
# =============================================================================

# architecture/platform translation to tool flags
# $(ASFLAGS) comes last so that nasm learns about the target platform first
ifeq ($(ARCH), x64)
	ifeq ($(PLATFORM), linux)
		REAL_ASFLAGS=-f elf64 $(ASFLAGS)
		REAL_CFLAGS=$(CFLAGS) -march=core2 -m64
		REAL_LDFLAGS=$(LDFLAGS) -melf_x86_64
		DEBUG_FORMAT=dwarf
		SO_EXT=.so
	else
		REAL_ASFLAGS=-f win64 $(ASFLAGS)
		REAL_CFLAGS=$(CFLAGS) -march=core2 -m64
		REAL_LDFLAGS=$(LDFLAGS) -mi386pep
		DEBUG_FORMAT=null
		SO_EXT=.dll
	endif
else
	ifeq ($(PLATFORM), linux)
		REAL_ASFLAGS=-f elf32 $(ASFLAGS)
		REAL_CFLAGS=$(CFLAGS) -march=pentium-m -m32
		REAL_LDFLAGS=$(LDFLAGS) -melf_i386
		DEBUG_FORMAT=dwarf
		SO_EXT=.so
	else
		REAL_ASFLAGS=-f win32 $(ASFLAGS)
		REAL_CFLAGS=$(CFLAGS) -march=pentium-m -m32
		REAL_LDFLAGS=$(LDFLAGS) -mi386pe
		DEBUG_FORMAT=null
		SO_EXT=.dll
	endif
endif

OUTPUT_FULL_NAME=$(OUTPUT_NAME)-$(PLATFORM)-$(ARCH)$(SO_EXT)

all: release

clean:
	rm -f *.o *.so

# recursively call make with proper flags
release:
	$(MAKE) clean
	$(MAKE) libparticlasm ARCH=$(ARCH) ASFLAGS="$(ASFLAGS) -Ox" CFLAGS="$(CFLAGS) -O3" LDFLAGS="$(LDFLAGS) -S"
	$(MKDIR_P) $(RELEASE_DIST_DIR)
	mv $(OUTPUT_FULL_NAME) $(RELEASE_DIST_DIR)

# recursively call make with proper flags
debug:
	$(MAKE) clean
	$(MAKE) libparticlasm ARCH=$(ARCH) ASFLAGS="$(ASFLAGS) -O0 -g -F $(DEBUG_FORMAT)" CFLAGS="$(CFLAGS) -O0 -g" LDFLAGS="$(LDFLAGS)"
	$(MKDIR_P) $(DEBUG_DIST_DIR)
	mv $(OUTPUT_FULL_NAME) $(DEBUG_DIST_DIR)

debug-linux-x86:
	$(MAKE) debug ARCH=x86 PLATFORM=linux

release-linux-x86:
	$(MAKE) release ARCH=x86 PLATFORM=linux

debug-linux-x64:
	$(MAKE) debug ARCH=x64 PLATFORM=linux

release-linux-x64:
	$(MAKE) release ARCH=x64 PLATFORM=linux

debug-windows-x86:
	$(MAKE) debug ARCH=x86 PLATFORM=windows

release-windows-x86:
	$(MAKE) release ARCH=x86 PLATFORM=windows

debug-windows-x64:
	$(MAKE) debug ARCH=x64 PLATFORM=windows

release-windows-x64:
	$(MAKE) release ARCH=x64 PLATFORM=windows

debug-mingw-x86:
	$(MAKE) debug ARCH=x86 PLATFORM=windows CC=i686-w64-mingw32-gcc

release-mingw-x86:
	$(MAKE) release ARCH=x86 PLATFORM=windows CC=i686-w64-mingw32-gcc

debug-mingw-x64:
	$(MAKE) debug ARCH=x64 PLATFORM=windows CC=x86_64-w64-mingw32-gcc

release-mingw-x64:
	$(MAKE) release ARCH=x64 PLATFORM=windows CC=x86_64-w64-mingw32-gcc

libparticlasm: particlasm.o ptc_highlevel.o
	$(LD) $(REAL_LDFLAGS) particlasm.o ptc_highlevel.o -o $(OUTPUT_FULL_NAME)

particlasm.o: particl.asm ptc_modules.inc ptc_distributions.inc libparticlasm.inc
	echo "Assembling..."
	$(AS) $(REAL_ASFLAGS) particl.asm -o particlasm.o

ptc_highlevel.o: ptc_highlevel.c libparticlasm.h
	echo "Compiling high level code..."
	$(CC) $(REAL_CFLAGS) ptc_highlevel.c -o ptc_highlevel.o

libparticlasm.inc: libparticlasm.h gen_asm_decls.py
	$(PYTHON) gen_asm_decls.py
