# libparticlasm makefile
# Copyright (C) 2011-2012, Leszek Godlewski <lg@ineqution.org>

REAL_OBJ_DIR=../../../$(OBJ_DIR)/Core
SOURCES=ParticlasmMain.cpp CodeGeneratorInterface.cpp
HEADERS=../../include/libparticlasm2.h ParticlasmMain.h CodeGeneratorInterface.h LauncherInterface.h
OBJECTS=$(addprefix $(REAL_OBJ_DIR)/,$(SOURCES:.cpp=.o))

all:
	@echo Please run 'make' in the libparticlasm directory.

build: $(OBJECTS)

$(OBJECTS): $(HEADERS) $(SOURCES)
	$(CXX) $(CXXFLAGS) -I../../include $(basename $(notdir $@)).cpp -o $(REAL_OBJ_DIR)/$@
