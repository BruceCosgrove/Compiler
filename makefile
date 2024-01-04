# Directory where all source files are located.
SRC_DIR := src/
# Directory where all intermediate files should be placed.
OBJ_DIR := bin-int/
# Directory where all output files should be placed.
OUT_DIR := bin/
# Output name.
OUT_NAME := shl
# The name of this makefile.
MAKEFILE := makefile

TEST_DIR := test/

# Get all source filepaths in $(SRC_DIR) and all subdirectories of $(SRC_DIR).
SRC := $(wildcard $(SRC_DIR)**.cpp) $(wildcard $(SRC_DIR)**/*.cpp)
# Get each obj filepath corresponding to each source filepath.
OBJ := $(patsubst $(SRC_DIR)%.cpp,$(OBJ_DIR)%.o,$(SRC))
# Get each dependency filepath corresponding to each source filepath.
DEP := $(patsubst $(SRC_DIR)%.cpp,$(OBJ_DIR)%.d,$(SRC))
# Get the executable's filepath.
EXE := $(OUT_DIR)$(OUT_NAME)
# Get all directories that should be pre-created (w/ duplicates removed so mkdir doesn't warn).
DIRS := $(sort $(OBJ_DIR) $(OUT_DIR) $(patsubst $(SRC_DIR)%,$(OBJ_DIR)%,$(sort $(dir $(SRC)))))

COMPILER := g++-13
CXXFLAGS := -std=c++23 -Wall

# Compile the executable.
all: $(EXE) $(MAKEFILE)

# Include all source dependencies.
-include $(DEP)

# Compile, get dependencies, and recompile everything if the makefile changed.
$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp $(MAKEFILE) | create_dirs
	$(COMPILER) $(CXXFLAGS) -MMD -o $@ -c $<

# Link all objs into executable.
$(EXE): $(OBJ) $(MAKEFILE)
	$(COMPILER) -o $(EXE) $(OBJ)

.PHONY: create_dirs
create_dirs: $(DIRS)
$(DIRS):
	mkdir -p $(DIRS)

.PHONY: run
run: $(EXE)
	$(MAKE) -C $(TEST_DIR) run

.PHONY: clean
clean:
	rm -rf $(OUT_DIR) $(OBJ_DIR)
	$(MAKE) -C $(TEST_DIR) clean
