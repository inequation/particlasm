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

$(OBJECTS): $(HEADERS) $(SOURCES)
	$(CXX) $(CXXFLAGS) -I../../include $(basename $(notdir $@)).cpp -o $(REAL_OBJ_DIR)/$@

AsmSnippets.h: GenerateSnippetHeader.py AsmSnippets/libparticlasm.inc $(SOURCES_ASM)
	$(PYTHON) GenerateSnippetHeader.py

AsmSnippets/libparticlasm.inc: GenerateAsmInclude.py ../../include/libparticlasm2.h
	$(PYTHON) GenerateAsmInclude.py
