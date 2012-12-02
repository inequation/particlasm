# libparticlasm build configuration
# Copyright (C) 2011-2012, Leszek Godlewski <lg@ineqution.org>

# =============================================================================
# Configuration
# Feel free to change any of these, as long as you know what you're doing.
# =============================================================================

# default build architecture: "x86" for 32-bit, "x64" for 64-bit
export ARCH=x64
# default platform: "windows" or "linux"
export PLATFORM=linux
# target platforms to build generators and launchers for
export TARGETS=RuntimeInterpreter X86Assembly

# mkdir command with arguments to create all missing directories in the path
export MKDIR_P=mkdir -p
# path to the Python interpreter
export PYTHON=python
# path to the nasm executable
export AS=nasm
# additional nasm flags
export ASFLAGS=
# path to the C++ compiler
export CXX=g++
# additional CXX flags
export CXXFLAGS=-c -Wall -Wextra -mfpmath=sse
# path to the linker executable
export LD=g++
# additional linker flags
export LDFLAGS=-shared -L/usr/lib32
