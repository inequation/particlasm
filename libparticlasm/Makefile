# libparticlasm makefile
# Copyright (C) 2011-2012, Leszek Godlewski <lg@ineqution.org>

# =============================================================================
# Configuration
# Feel free to change any of these, as long as you know what you're doing.
# =============================================================================

# path to make
MAKE = make
# mkdir command with arguments to create all missing directories in the path
MKDIR_P = mkdir -p
# path to the Python interpreter
PYTHON = python
# path to the nasm executable
AS = nasm
# additional nasm flags
ASFLAGS = -f elf
# path to the C compiler
CC = gcc
# additional CC flags
CFLAGS = -fPIC -c -Wall -Wextra -march=pentium-m -mfpmath=sse
# path to the linker executable
LINKER = ld
# additional linker flags
LINKERFLAGS = -shared
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
	$(MAKE) libparticlasm.so ASFLAGS="$(ASFLAGS) -Ox" CFLAGS="$(CFLAGS) -O3" LINKERFLAGS="$(LINKERFLAGS) -S"
	$(MKDIR_P) $(RELEASE_DIST_DIR)
	mv libparticlasm.so $(RELEASE_DIST_DIR)

# recursively call make with proper flags
debug:
	$(MAKE) clean
	$(MAKE) libparticlasm.so ASFLAGS="$(ASFLAGS) -O0 -g -F dwarf" CFLAGS="$(CFLAGS) -O0 -g" LINKERFLAGS="$(LINKERFLAGS)"
	$(MKDIR_P) $(DEBUG_DIST_DIR)
	mv libparticlasm.so $(DEBUG_DIST_DIR)

libparticlasm.so: particlasm.o ptc_highlevel.o
	$(LINKER) $(LINKERFLAGS) particlasm.o ptc_highlevel.o -o libparticlasm.so

particlasm.o: particl.asm ptc_modules.inc ptc_distributions.inc libparticlasm.inc
	echo "Assembling..."
	$(AS) $(ASFLAGS) particl.asm -o particlasm.o

ptc_highlevel.o: ptc_highlevel.c libparticlasm.h
	echo "Compiling high level code..."
	$(CC) $(CFLAGS) ptc_highlevel.c -o ptc_highlevel.o

libparticlasm.inc: libparticlasm.h gen_asm_decls.py
	$(PYTHON) gen_asm_decls.py
