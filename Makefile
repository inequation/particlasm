# Global Particlasm makefile
# Copyright (C) 2011-2012, Leszek Godlewski <lg@ineqution.org>

# =============================================================================
# Configuration
# Feel free to change any of these, as long as you know what you're doing.
# =============================================================================

# path to make
MAKE = make
# path to Doxygen
DOXYGEN = doxygen

# =============================================================================

all: libparticlasm hostapp headless doc

libparticlasm:
	cd libparticlasm
	$(MAKE)

hostapp:
	cd hostapp
	$(MAKE)

headless:
	cd headless
	$(MAKE)

doc:
	cd doc
	$(DOXYGEN)