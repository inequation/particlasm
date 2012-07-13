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
CXXFLAGS = -fPIC -Wall -Wextra -mfpmath=sse
# additional linker flags
LINKERFLAGS =
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

SOURCES = main.cpp

all: release

clean:
	rm -f *.o

# recursively call make with proper flags
release:
	$(MAKE) edificle CXXFLAGS="$(CFLAGS) -O3" LINKERFLAGS="$(LINKERFLAGS) -S" WX_CXXFLAGS="`wx-config  --version=2.8 --static=no --unicode=yes --debug=no --cflags`" WX_LINKERFLAGS="`wx-config  --version=2.8 --static=no --unicode=yes --debug=no --libs`"
	$(MKDIR_P) $(RELEASE_DIST_DIR)
	mv edificle$(EXT) $(RELEASE_DIST_DIR)

# recursively call make with proper flags
debug:
	$(MAKE) edificle CXXFLAGS="$(CFLAGS) -O0 -g" LINKERFLAGS="$(LINKERFLAGS)" WX_CXXFLAGS="`wx-config  --version=2.8 --static=no --unicode=yes --debug=yes --cflags`" WX_LINKERFLAGS="`wx-config  --version=2.8 --static=no --unicode=yes --debug=yes --libs`"
	$(MKDIR_P) $(DEBUG_DIST_DIR)
	mv edificle$(EXT) $(DEBUG_DIST_DIR)

edificle: $(SOURCES)
	echo "Compiling high level code..."
	$(CXX) $(CXXFLAGS) $(WX_CXXFLAGS) $(LINKERFLAGS) $(WX_LINKERFLAGS) $(SOURCES) -o edificle$(EXEC_EXT)
