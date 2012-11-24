# Particlasm headless benchmark makefile
# Copyright (C) 2012, Leszek Godlewski <lg@ineqution.org>

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
# path to the C++ compiler
CXX=g++
# additional CC flags
CXXFLAGS=-Wall -Wextra -mfpmath=sse -I../libparticlasm/include
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
		REAL_CXXFLAGS=$(CXXFLAGS) -march=core2 -m64 -Wl,-m64,-melf_x86_64 -ldl
		EXEC_EXT=
	else
		REAL_CXXFLAGS = $(CXXFLAGS) -march=core2 -m64 -Wl,-m64,-mi386pep
		EXEC_EXT=.exe
	endif
else
	ifeq ($(PLATFORM),linux)
		REAL_CXXFLAGS=$(CXXFLAGS) -march=pentium-m -m32 -Wl,-m32,-melf_i386 -ldl
		EXEC_EXT=
	else
		REAL_CXXFLAGS=$(CXXFLAGS) -march=pentium-m -m32 -Wl,-m32,-mi386pe
		EXEC_EXT=.exe
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
	$(MAKE) debug ARCH=x86 PLATFORM=windows CXX=i686-w64-mingw32-g++

release-mingw-x86:
	$(MAKE) release ARCH=x86 PLATFORM=windows CXX=i686-w64-mingw32-g++

debug-mingw-x64:
	$(MAKE) debug ARCH=x64 PLATFORM=windows CXX=x86_64-w64-mingw32-g++

release-mingw-x64:
	$(MAKE) release ARCH=x64 PLATFORM=windows CXX=x86_64-w64-mingw32-g++

headless:
	$(CXX) $(REAL_CXXFLAGS) $(REAL_LDFLAGS) -o $(OUTPUT_FULL_NAME) -DARCH=\"$(ARCH)\" -DPLATFORM=\"$(PLATFORM)\" headless.cpp ../hostapp/emitters.cpp ../libparticlasm/src/ReferenceImplementation.cpp
