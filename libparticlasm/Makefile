# libparticlasm makefile
# Copyright (C) 2011-2012, Leszek Godlewski <lg@ineqution.org>

# =============================================================================
# Configuration
# Feel free to change any of these, as long as you know what you're doing.
# =============================================================================

# architecture: "x86" for 32-bit, "x64" for 64-bit
ARCH = x64
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
	$(MAKE) libparticlasm.so ARCH=$(ARCH) ASFLAGS="$(ASFLAGS) -O0 -g" CFLAGS="$(CFLAGS) -O0 -g" LINKERFLAGS="$(LINKERFLAGS)"
	$(MKDIR_P) $(DEBUG_DIST_DIR)
	mv libparticlasm.so $(DEBUG_DIST_DIR)

libparticlasm.so: particlasm.o ptc_highlevel.o
ifeq ($(ARCH), x64)
	$(LINKER) $(LINKERFLAGS) -melf_x86_64 particlasm.o ptc_highlevel.o -o libparticlasm.so
else
	$(LINKER) $(LINKERFLAGS) -melf_i386 particlasm.o ptc_highlevel.o -o libparticlasm.so
endif

particlasm.o: particl.asm ptc_modules.inc ptc_distributions.inc libparticlasm.inc
	echo "Assembling (ARCH=$(ARCH))..."
ifeq ($(ARCH), x64)
	$(AS) $(ASFLAGS) -dPTC_ARCH=X64 -f elf64 particl.asm -o particlasm.o
else
	$(AS) $(ASFLAGS) -dPTC_ARCH=P3 -f elf32 particl.asm -o particlasm.o
endif

ptc_highlevel.o: ptc_highlevel.c libparticlasm.h
	echo "Compiling high level code..."
ifeq ($(ARCH), x64)
	$(CC) $(CFLAGS) -march=core2 -m64 ptc_highlevel.c -o ptc_highlevel.o
else
	$(CC) $(CFLAGS) -march=pentium-m -m32 ptc_highlevel.c -o ptc_highlevel.o
endif

libparticlasm.inc: libparticlasm.h gen_asm_decls.py
	$(PYTHON) gen_asm_decls.py
