# The name of this makefile.
MAKEFILE := makefile
# The output name.
OUT_NAME := shl
# The directory where the test is.
TEST_DIR := test/
# The default build configuration.
# To set directly, use "make cfg=debug" or "make cfg=release".
cfg := debug

# The compiler flags to use.
CXXFLAGS := -std=c++23 -Wall
CXXFLAGS.debug := -g
CXXFLAGS.release := -O2

# The directory where all source files are located.
SRC_DIR := src/
# The directory where all intermediate files should be placed.
INT_DIR_ROOT := bin-int/
INT_DIR := $(INT_DIR_ROOT)$(cfg)/
# The directory where all output files should be placed.
OUT_DIR_ROOT := bin/
OUT_DIR := $(OUT_DIR_ROOT)$(cfg)/

# Get all source filepaths in $(SRC_DIR) and all subdirectories of $(SRC_DIR).
SRCS := $(wildcard $(SRC_DIR)*.cpp) $(wildcard $(SRC_DIR)**/*.cpp)
# Get each obj filepath corresponding to each source filepath.
OBJS := $(patsubst $(SRC_DIR)%.cpp,$(INT_DIR)%.o,$(SRCS))
# Get each dependency filepath corresponding to each source filepath.
DEPS := $(patsubst $(SRC_DIR)%.cpp,$(INT_DIR)%.d,$(SRCS))
# Get all directories that should be pre-created (w/ duplicates removed so mkdir doesn't warn).
DIRS := $(sort $(INT_DIR) $(OUT_DIR) $(patsubst $(SRC_DIR)%,$(INT_DIR)%,$(sort $(dir $(SRCS)))))
# Get the executable's filepath.
EXE := $(OUT_DIR)$(OUT_NAME)

# The compiler to use.
COMPILER := g++-13
# Add the configuration-specific compiler flags.
CXXFLAGS += $(CXXFLAGS.$(cfg))

# Compile the executable.
all: $(EXE) $(MAKEFILE)

# Include all source dependencies.
-include $(DEPS)

# Create necessary directories, compile, get dependencies, and make sure to recompile everything if the makefile changed.
$(INT_DIR)%.o: $(SRC_DIR)%.cpp $(MAKEFILE) | create_dirs
	$(COMPILER) $(CXXFLAGS) -MMD -c -o $@ $<

# Link all objs into the executable.
$(EXE): $(OBJS) $(MAKEFILE)
	$(COMPILER) -o $(EXE) $(OBJS)

# Creates all the necessary directories.
.PHONY: create_dirs
create_dirs: $(DIRS)
$(DIRS):
	mkdir -p $(DIRS)

# Runs the test.
.PHONY: run
run: $(EXE)
	$(MAKE) cfg=$(cfg) -C $(TEST_DIR) run

# Clears the terminal and compiles the necessary files.
# If successful, also runs the test.
.PHONY: crun
crun: clean run

# Clears the terminal and recompiles everything.
# If successful, also runs the test.
.PHONY: ccrun
ccrun: clear clean run

# Debugs the test.
.PHONY: rund
rund: $(EXE)
	$(MAKE) cfg=$(cfg) -C $(TEST_DIR) rund

# Removes all intermediates and the executable.
.PHONY: clean
clean:
	rm -rf $(OUT_DIR_ROOT) $(INT_DIR_ROOT)
	$(MAKE) cfg=$(cfg) -C $(TEST_DIR) clean

# Clears the terminal.
.PHONY: clear
clear:
	clear
