# libparticlasm makefile
# Copyright (C) 2011-2012, Leszek Godlewski <lg@ineqution.org>

SUBDIRS=Core $(TARGETS)

.PHONY: $(SUBDIRS)

all:
	@echo Please run 'make' in the libparticlasm directory.

clean:
	-cd ../../$(OBJ_DIR) && rm -rf $(SUBDIRS)

build: $(SUBDIRS)

$(SUBDIRS):
	$(MKDIR_P) ../../$(OBJ_DIR)/$@
	cd $@ && $(MAKE) build
