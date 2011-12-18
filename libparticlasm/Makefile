# libparticlasm makefile
# Copyright (C) 2011, Leszek Godlewski <lg@ineqution.org>

# =============================================================================
# Configuration
# Feel free to change any of these, as long as you know what you're doing.
# =============================================================================

# path to make
MAKE = make
# mkdir command with arguments to create all missing directories in the path
MKDIR_P = mkdir -p
# path to the nasm executable
AS = nasm
# additional nasm flags
ASFLAGS = -f elf
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
	rm *.o *.so

# recursively call make with proper flags
release:
	$(MAKE) libparticlasm.so ASFLAGS="$(ASFLAGS) -Ox" LINKERFLAGS="$(LINKERFLAGS) -S"
	$(MKDIR_P) $(RELEASE_DIST_DIR)
	mv libparticlasm.so $(RELEASE_DIST_DIR)

# recursively call make with proper flags
debug:
	$(MAKE) libparticlasm.so ASFLAGS="$(ASFLAGS) -O0 -g -F dwarf" LINKERFLAGS="$(LINKERFLAGS)"
	$(MKDIR_P) $(DEBUG_DIST_DIR)
	mv libparticlasm.so $(DEBUG_DIST_DIR)

libparticlasm.so: particlasm.o ptc_modules.o
	$(LINKER) $(LINKERFLAGS) particlasm.o ptc_modules.o -o libparticlasm.so

particlasm.o: particl.asm
	$(AS) $(ASFLAGS) particl.asm -o particlasm.o

ptc_modules.o: ptc_modules.asm
	$(AS) $(ASFLAGS) ptc_modules.asm -o ptc_modules.o
