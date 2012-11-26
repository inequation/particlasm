# libparticlasm makefile
# Copyright (C) 2011-2012, Leszek Godlewski <lg@ineqution.org>

# =============================================================================
# Configuration
# =============================================================================

# users may edit this file
include config.mk

# =============================================================================
# End of configuration
# =============================================================================

# architecture/platform translation to tool flags
# $(ASFLAGS) comes last so that nasm learns about the target platform first
ifeq ($(ARCH), x64)
	ifeq ($(PLATFORM), linux)
		ASFLAGS:=-f elf64 $(ASFLAGS)
		CXXFLAGS:=-fPIC $(CXXFLAGS) -march=core2 -m64
		LDFLAGS:=$(LDFLAGS) -m64 -Wl,-melf_x86_64
		DEBUG_FORMAT=dwarf
		SO_EXT=.so
	else
		ASFLAGS:=-f win64 $(ASFLAGS)
		CXXFLAGS:=$(CXXFLAGS) -march=core2 -m64
		LDFLAGS:=$(LDFLAGS) -m64 -Wl,-mi386pep
		DEBUG_FORMAT=null
		SO_EXT=.dll
	endif
else
	ifeq ($(PLATFORM), linux)
		ASFLAGS:=-f elf32 $(ASFLAGS)
		CXXFLAGS:=-fPIC $(CXXFLAGS) -march=pentium-m -m32
		LDFLAGS:=$(LDFLAGS) -m32 -Wl,-melf_i386
		DEBUG_FORMAT=dwarf
		SO_EXT=.so
	else
		ASFLAGS:=-f win32 $(ASFLAGS)
		CXXFLAGS:=$(CXXFLAGS) -march=pentium-m -m32
		LDFLAGS:=$(LDFLAGS) -m32 -Wl,-mi386pe
		DEBUG_FORMAT=null
		SO_EXT=.dll
	endif
endif

# debug dist directory
DEBUG_DIST_DIR=bin/Debug
# debug object directory
DEBUG_OBJ_DIR=obj/Debug
# release dist directory
RELEASE_DIST_DIR=bin/Release
# release object directory
RELEASE_OBJ_DIR=obj/Release
# additional assembler flags for debug builds
DEBUG_ASFLAGS= -O0 -g -F $(DEBUG_FORMAT)
# additional assembler flags for release builds
RELEASE_ASFLAGS= -Ox
# additional C++ compiler flags for debug builds
DEBUG_CXXFLAGS= -O0 -g
# additional C++ compiler flags for release builds
RELEASE_CXXFLAGS= -O3 -DNDEBUG
# additional linker flags for debug builds
DEBUG_LDFLAGS=
# additional linker flags for release builds
RELEASE_LDFLAGS= -S
# output library file name
OUTPUT_NAME=libparticlasm2

OUTPUT_FULL_NAME=$(OUTPUT_NAME)-$(PLATFORM)-$(ARCH)$(SO_EXT)
OUTPUT_FULL_PATH=../$(DIST_DIR)/$(OUTPUT_FULL_NAME)
TARGET_OBJS=$(addsuffix /*.o,$(addprefix ../$(OBJ_DIR)/,Core $(TARGETS)))

# FIXME! figure out the OUTPUT_FULL_PATH's dependency on the objects
.PHONY: all clean release debug build objects $(OUTPUT_FULL_PATH) $(TARGET_OBJS)

all: release

clean:
	cd src && $(MAKE) clean OBJ_DIR=$(DEBUG_OBJ_DIR)
	cd src && $(MAKE) clean OBJ_DIR=$(RELEASE_OBJ_DIR)
	-rm -rf ../$(DEBUG_DIST_DIR) ../$(RELEASE_DIST_DIR)

# recursively call make with proper flags
release:
	$(MAKE) build OBJ_DIR=$(RELEASE_OBJ_DIR) DIST_DIR=$(RELEASE_DIST_DIR) ASFLAGS="$(ASFLAGS) $(RELEASE_ASFLAGS)" CXXFLAGS="$(CXXFLAGS) $(RELEASE_CXXFLAGS)" LDFLAGS="$(LDFLAGS) $(RELEASE_LDFLAGS)"

debug:
	$(MAKE) build OBJ_DIR=$(DEBUG_OBJ_DIR) DIST_DIR=$(DEBUG_DIST_DIR) ASFLAGS="$(ASFLAGS) $(DEBUG_ASFLAGS)" CXXFLAGS="$(CXXFLAGS) $(DEBUG_CXXFLAGS)" LDFLAGS="$(LDFLAGS) $(DEBUG_LDFLAGS)"

# recursively call make with proper flags
build: $(OUTPUT_FULL_PATH)

$(OUTPUT_FULL_PATH): $(TARGET_OBJS)
	$(MKDIR_P) ../$(DIST_DIR)
	$(MKDIR_P) ../$(OBJ_DIR)
	cd src && $(MAKE) build
	# FIXME! figure out how to link only when needed
	$(LD) $(LDFLAGS) $(TARGET_OBJS) -o $@

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
	$(MAKE) debug ARCH=x86 PLATFORM=windows CXX=i686-w64-mingw32-g++ LD=i686-w64-mingw32-g++ PATH_SEPARATOR=/

release-mingw-x86:
	$(MAKE) release ARCH=x86 PLATFORM=windows CXX=i686-w64-mingw32-g++ LD=i686-w64-mingw32-g++ PATH_SEPARATOR=/

debug-mingw-x64:
	$(MAKE) debug ARCH=x64 PLATFORM=windows CXX=x86_64-w64-mingw32-g++ LD=x86_64-w64-mingw32-g++ PATH_SEPARATOR=/

release-mingw-x64:
	$(MAKE) release ARCH=x64 PLATFORM=windows CXX=x86_64-w64-mingw32-g++ LD=x86_64-w64-mingw32-g++ PATH_SEPARATOR=/

