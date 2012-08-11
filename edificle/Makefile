# edificle makefile
# Copyright (C) 2012, Leszek Godlewski <lg@ineqution.org>

# =============================================================================
# Configuration
# Feel free to change any of these, as long as you know what you're doing.
# =============================================================================

# path to make
MAKE = make
# mkdir command with arguments to create all missing directories in the path
MKDIR_P = mkdir -p
# path to the C++ compiler
CXX = g++
# additional CXX flags
CXXFLAGS = -fPIC -Wall -Wextra -msse -mfpmath=sse -std=gnu++0x
# additional linker flags
LDFLAGS =
# debug dist directory
DEBUG_DIST_DIR = ../bin/Debug
# release dist directory
RELEASE_DIST_DIR = ../bin/Release
# executable file extension
EXEC_EXT =
#EXEC_EXT =.exe

# =============================================================================
# End of configuration
# =============================================================================

SOURCES=main.cpp MainFrame.cpp
OBJECTS=$(SOURCES:.cpp=.o)

all: release

clean: cleandebug cleanrelease

cleanrelease:
	rm -f *.o $(RELEASE_DIST_DIR)/edificle$(EXEC_EXT)

cleandebug:
	rm -f *.o $(DEBUG_DIST_DIR)/edificle$(EXEC_EXT)

# recursively call make with proper flags
release:
	$(MKDIR_P) $(RELEASE_DIST_DIR)
	$(MAKE) $(RELEASE_DIST_DIR)/edificle$(EXEC_EXT) DIST_DIR="$(RELEASE_DIST_DIR)/"  CXXFLAGS="$(CXXFLAGS) -O3" LDFLAGS="$(LDFLAGS) -s -O3 --relax" WX_CXXFLAGS="`wx-config  --version=2.8 --static=no --unicode=yes --debug=no --cflags`" WX_LDFLAGS="`wx-config  --version=2.8 --static=no --unicode=yes --debug=no --libs`"
	mv edificle$(EXEC_EXT) $(RELEASE_DIST_DIR)

# recursively call make with proper flags
debug:
	$(MKDIR_P) $(DEBUG_DIST_DIR)
	$(MAKE) $(DEBUG_DIST_DIR)/edificle$(EXEC_EXT) DIST_DIR="$(DEBUG_DIST_DIR)/" CXXFLAGS="$(CXXFLAGS) -O0 -g" LDFLAGS="$(LDFLAGS)" WX_CXXFLAGS="`wx-config  --version=2.8 --static=no --unicode=yes --debug=yes --cflags`" WX_LDFLAGS="`wx-config  --version=2.8 --static=no --unicode=yes --debug=yes --libs`"

$(DIST_DIR)edificle$(EXEC_EXT): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(WX_LDFLAGS) $(OBJECTS) -o $(DIST_DIR)edificle$(EXEC_EXT)

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(WX_CXXFLAGS) $< -o $@
