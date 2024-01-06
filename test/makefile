# The name of this makefile.
MAKEFILE := makefile
# The directory where all source files are located.
SRC_DIR := src/
# The directory where all intermediate files should be placed.
INT_DIR := bin-int/
# The directory where all output files should be placed.
OUT_DIR := bin/
# The output name.
OUT_NAME := test

# Get the source's filepath.
SRC := $(SRC_DIR)$(OUT_NAME).shl
# Get the assembly's filepath.
ASM := $(INT_DIR)$(OUT_NAME).asm
# Get the obj's filepath.
OBJ := $(INT_DIR)$(OUT_NAME).o
# Get all directories that should be pre-created (w/ duplicates removed so mkdir doesn't warn).
DIRS := $(sort $(INT_DIR) $(OUT_DIR))
# Get the executable's filepath.
EXE := $(OUT_DIR)$(OUT_NAME)

# The compiler to use.
COMPILER := ../bin/shl
# The whole compilation command.
COMPILE := $(COMPILER) $(SRC) -o $(ASM)

# Compile the executable.
all: $(EXE) $(MAKEFILE)

# Create necessary directories, compile, and make sure to recompile if the makefile or compiler changed.
$(ASM): $(SRC) $(MAKEFILE) $(COMPILER) | create_dirs
	$(COMPILE)

# Assemble
$(OBJ): $(ASM) $(MAKEFILE)
	nasm -f elf64 $< -o $@

# Link
$(EXE): $(OBJ) $(MAKEFILE)
	ld $< -o $@

# Creates all the necessary directories.
.PHONY: create_dirs
create_dirs: $(DIRS)
$(DIRS):
	mkdir -p $(DIRS)

# Runs the executable.
.PHONY: run
run: $(EXE)
	./$(EXE); echo $(EXE) exited with return code $$?.

# Debugs the executable.
.PHONY: rund
rund:
	gdb --args $(COMPILE)

# Removes all intermediates and the executable.
.PHONY: clean
clean:
	rm -rf $(OUT_DIR) $(INT_DIR)
