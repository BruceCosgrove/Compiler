SRC_DIR := src/
INT_DIR := bin-int/
OUT_DIR := bin/
NAME := test

SRC := $(SRC_DIR)$(NAME).shl
ASM := $(INT_DIR)$(NAME).asm
OBJ := $(INT_DIR)$(NAME).o
EXE := $(OUT_DIR)$(NAME)

COMPILER := ../bin/shl

all: $(EXE)

# Compile
$(ASM): $(SRC) | create_dirs
	$(COMPILER) $(SRC) $(ASM)

# Assemble
$(OBJ): $(ASM)
	nasm -f elf64 $< -o $@

# Link
$(EXE): $(OBJ)
	ld $< -o $@

.PHONY: create_dirs
create_dirs: $(INT_DIR) $(OUT_DIR)
$(INT_DIR) $(OUT_DIR):
	mkdir -p $(INT_DIR) $(OUT_DIR)

.PHONY: run
run: $(EXE)
	./$(EXE); echo $(EXE) exited with return code $$?.

.PHONY: clean
clean:
	rm -rf $(INT_DIR) $(OUT_DIR)
