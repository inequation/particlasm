# libparticlasm makefile
# Copyright (C) 2011-2012, Leszek Godlewski <lg@ineqution.org>

REAL_OBJ_DIR=../../../$(OBJ_DIR)/X86Assembly
SOURCES=$(wildcard *.cpp)
SOURCES_ASM=$(wildcard AsmSnippets/*.asm) $(wildcard AsmSnippets/*.inc)
HEADERS=../../include/libparticlasm2.h AsmSnippets.h $(wildcard *.h)
OBJECTS=$(addprefix $(REAL_OBJ_DIR)/,$(SOURCES:.cpp=.o))

all:
	@echo Please run 'make' in the libparticlasm directory.

build: AsmSnippets.h $(OBJECTS)

# HACK!!! https://github.com/inequation/particlasm/issues/2
# The presence of the red zone breaks X86Launcher::ProcessParticles because gcc
# is not aware of the assembly code's stack access; as a result, the assembly
# code overwrites the function parameters that gcc puts in the red zone. As a
# temporary alleviation, I'm disabling the red zone altogether.
$(OBJECTS): $(HEADERS) $(SOURCES)
	$(CXX) $(CXXFLAGS) -mno-red-zone -I../../include $(basename $(notdir $@)).cpp -o $@

AsmSnippets.h: GenerateSnippetHeader.py AsmSnippets/libparticlasm.inc $(SOURCES_ASM)
	$(PYTHON) GenerateSnippetHeader.py

AsmSnippets/libparticlasm.inc: GenerateAsmInclude.py ../../include/libparticlasm2.h
	$(PYTHON) GenerateAsmInclude.py
