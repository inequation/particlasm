# Particlasm headless benchmark makefile
# Copyright (C) 2012, Leszek Godlewski <lg@ineqution.org>

# =============================================================================
# Configuration
# Feel free to change any of these, as long as you know what you're doing.
# =============================================================================

# architecture: "x86" for 32-bit, "x64" for 64-bit
ARCH=x64
# platform: "windows" or "linux"
PLATFORM=linux
# path to make
MAKE=make
# mkdir command with arguments to create all missing directories in the path
MKDIR_P=mkdir -p
# path to the C++ compiler
CXX=g++
# additional CC flags
CXXFLAGS=-Wall -Wextra -mfpmath=sse -ldl
# path to the linker executable
LD=ld
# additional linker flags
LDFLAGS=-shared -lc
# debug dist directory
DEBUG_DIST_DIR=../bin/Debug
# release dist directory
RELEASE_DIST_DIR=../bin/Release
# output executable file name
OUTPUT_NAME=headless

# =============================================================================
# End of configuration
# =============================================================================

# architecture/platform translation to tool flags
# $(ASFLAGS) comes last so that nasm learns about the target platform first
ifeq ($(ARCH), x64)
	ifeq ($(PLATFORM),linux)
		REAL_CXXFLAGS=$(CXXFLAGS) -march=core2 -m64
		EXEC_EXT=
		SO_EXT=.so
	else
		REAL_CXXFLAGS = $(CXXFLAGS) -march=core2 -m64
		EXEC_EXT=.exe
		SO_EXT=.dll
	endif
else
	ifeq ($(PLATFORM),linux)
		REAL_CXXFLAGS=$(CXXFLAGS) -march=pentium-m -m32
		EXEC_EXT=
		SO_EXT=.so
	else
		REAL_CXXFLAGS=$(CXXFLAGS) -march=pentium-m -m32
		EXEC_EXT=.exe
		SO_EXT=.dll
	endif
endif

OUTPUT_FULL_NAME=$(OUTPUT_NAME)-$(PLATFORM)-$(ARCH)$(EXEC_EXT)

all: release

clean:
	rm -f *.o *.so $(RELEASE_DIST_DIR)/$(OUTPUT_FULL_NAME) $(DEBUG_DIST_DIR)/$(OUTPUT_FULL_NAME)

# recursively call make with proper flags
release:
	$(MAKE) headless ARCH=$(ARCH) CXXFLAGS="$(CXXFLAGS) -O3"
	$(MKDIR_P) $(RELEASE_DIST_DIR)
	mv $(OUTPUT_FULL_NAME) $(RELEASE_DIST_DIR)

# recursively call make with proper flags
debug:
	$(MAKE) headless ARCH=$(ARCH) CXXFLAGS="$(CXXFLAGS) -O0 -g"
	$(MKDIR_P) $(DEBUG_DIST_DIR)
	mv $(OUTPUT_FULL_NAME) $(DEBUG_DIST_DIR)

headless:
	$(CXX) $(REAL_CXXFLAGS) $(REAL_LDFLAGS) -o $(OUTPUT_FULL_NAME) -DARCH=\"$(ARCH)\" -DPLATFORM=\"$(PLATFORM)\" -DSO_EXT=\"$(SO_EXT)\" headless.c ../hostapp/emitters.cpp ../libparticlasm/ref_impl.cpp
