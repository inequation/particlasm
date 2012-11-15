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
# path to GNU make
MAKE=make
# mkdir command with arguments to create all missing directories in the path
MKDIR_P=mkdir -p
# path to the Python interpreter
PYTHON=python
# path to the nasm executable
AS=nasm
# additional nasm flags
ASFLAGS=
# path to the C++ compiler
CXX=g++
# additional CXX flags
CXXFLAGS=-fPIC -c -Wall -Wextra -mfpmath=sse
# path to the linker executable
LD=g++
# additional linker flags
LDFLAGS=-shared -L/usr/lib32
# debug dist directory
DEBUG_DIST_DIR=../bin/Debug
# release dist directory
RELEASE_DIST_DIR=../bin/Release
# output library file name
OUTPUT_NAME=libparticlasm2

# =============================================================================
# End of configuration
# =============================================================================

# architecture/platform translation to tool flags
# $(ASFLAGS) comes last so that nasm learns about the target platform first
ifeq ($(ARCH), x64)
	ifeq ($(PLATFORM), linux)
		REAL_ASFLAGS=-f elf64 $(ASFLAGS)
		REAL_CXXFLAGS=$(CXXFLAGS) -march=core2 -m64
		REAL_LDFLAGS=$(LDFLAGS) -m64 -Wl,-melf_x86_64
		DEBUG_FORMAT=dwarf
		SO_EXT=.so
		PATH_SEPARATOR=/
	else
		REAL_ASFLAGS=-f win64 $(ASFLAGS)
		REAL_CXXFLAGS=$(CXXFLAGS) -march=core2 -m64
		REAL_LDFLAGS=$(LDFLAGS) -m64 -Wl,-mi386pep
		DEBUG_FORMAT=null
		SO_EXT=.dll
		PATH_SEPARATOR=\\
	endif
else
	ifeq ($(PLATFORM), linux)
		REAL_ASFLAGS=-f elf32 $(ASFLAGS)
		REAL_CXXFLAGS=$(CXXFLAGS) -march=pentium-m -m32
		REAL_LDFLAGS=$(LDFLAGS) -m32 -Wl,-melf_i386
		DEBUG_FORMAT=dwarf
		SO_EXT=.so
		PATH_SEPARATOR=/
	else
		REAL_ASFLAGS=-f win32 $(ASFLAGS)
		REAL_CXXFLAGS=$(CXXFLAGS) -march=pentium-m -m32
		REAL_LDFLAGS=$(LDFLAGS) -m32 -Wl,-mi386pe
		DEBUG_FORMAT=null
		SO_EXT=.dll
		PATH_SEPARATOR=\\
	endif
endif

OUTPUT_FULL_NAME=$(OUTPUT_NAME)-$(PLATFORM)-$(ARCH)$(SO_EXT)

SOURCES=ParticlasmMain.cpp libparticlasm.h CodeGeneratorInterface.h
OBJECTS=$(SOURCES:.cpp=.o)
SOURCES_X86_ASSEMBLY_DISTRIBS=$(wildcard X86Assembly$(PATH_SEPARATOR)Distr_*.cpp) $(wildcard X86Assembly$(PATH_SEPARATOR)Distr_*.h)
SOURCES_X86_ASSEMBLY_MODULES=$(wildcard X86Assembly$(PATH_SEPARATOR)Mod_*.cpp) $(wildcard X86Assembly$(PATH_SEPARATOR)Mod_*.h)
SOURCES_X86_ASSEMBLY_RAW=AsmSnippets$(PATH_SEPARATOR)AsmSnippets.h X86AssemblyGenerator.cpp X86AssemblyGenerator.h X86Module.cpp X86Module.h X86Distribution.h
SOURCES_X86_ASSEMBLY_SNIPPETS=$(wildcard X86Assembly$(PATH_SEPARATOR)AsmSnippets$(PATH_SEPARATOR)*.asm)
SOURCES_X86_ASSEMBLY=$(addprefix X86Assembly$(PATH_SEPARATOR),$(SOURCES_X86_ASSEMBLY_RAW)) $(SOURCES_X86_ASSEMBLY_DISTRIBS) $(SOURCES_X86_ASSEMBLY_MODULES)
OBJECTS_X86_ASSEMBLY=$(SOURCES_X86_ASSEMBLY:.cpp=.o)

all: release

clean:
	rm -f *.o *.so X86Assembly$(PATH_SEPARATOR)*.o X86Assembly$(PATH_SEPARATOR)AsmSnippets$(PATH_SEPARATOR)libparticlasm.inc

# recursively call make with proper flags
release:
	$(MAKE) clean
	$(MAKE) libparticlasm ARCH=$(ARCH) ASFLAGS="$(ASFLAGS) -Ox" CXXFLAGS="$(CXXFLAGS) -O3 -DNDEBUG" LDFLAGS="$(LDFLAGS) -S"
	$(MKDIR_P) $(RELEASE_DIST_DIR)
	mv $(OUTPUT_FULL_NAME) $(RELEASE_DIST_DIR)

# recursively call make with proper flags
debug:
	$(MAKE) clean
	$(MAKE) libparticlasm ARCH=$(ARCH) ASFLAGS="$(ASFLAGS) -O0 -g -F $(DEBUG_FORMAT)" CXXFLAGS="$(CXXFLAGS) -O0 -g" LDFLAGS="$(LDFLAGS)"
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
	$(MAKE) debug ARCH=x86 PLATFORM=windows CXX=i686-w64-mingw32-g++ PATH_SEPARATOR=/

release-mingw-x86:
	$(MAKE) release ARCH=x86 PLATFORM=windows CXX=i686-w64-mingw32-g++ PATH_SEPARATOR=/

debug-mingw-x64:
	$(MAKE) debug ARCH=x64 PLATFORM=windows CXX=x86_64-w64-mingw32-g++ PATH_SEPARATOR=/

release-mingw-x64:
	$(MAKE) release ARCH=x64 PLATFORM=windows CXX=x86_64-w64-mingw32-g++ PATH_SEPARATOR=/

libparticlasm: X86Assembly$(PATH_SEPARATOR)AsmSnippets$(PATH_SEPARATOR)AsmSnippets.h $(OBJECTS) $(OBJECTS_X86_ASSEMBLY)
	$(LD) $(REAL_LDFLAGS) $(OBJECTS) $(OBJECTS_X86_ASSEMBLY) -o $(OUTPUT_FULL_NAME)

.cpp.o: $(SOURCES) $(SOURCES_X86_ASSEMBLY)
	$(CXX) $(REAL_CXXFLAGS) $< -o $@

X86Assembly$(PATH_SEPARATOR)AsmSnippets$(PATH_SEPARATOR)libparticlasm.inc: libparticlasm.h X86Assembly$(PATH_SEPARATOR)gen_asm_decls.py
	$(PYTHON) X86Assembly$(PATH_SEPARATOR)gen_asm_decls.py

X86Assembly$(PATH_SEPARATOR)AsmSnippets$(PATH_SEPARATOR)AsmSnippets.h: X86Assembly$(PATH_SEPARATOR)AsmSnippets$(PATH_SEPARATOR)libparticlasm.inc $(SOURCES_X86_ASSEMBLY_SNIPPETS) X86Assembly$(PATH_SEPARATOR)gen_asm_snippets.py
	$(PYTHON) X86Assembly$(PATH_SEPARATOR)gen_asm_snippets.py
