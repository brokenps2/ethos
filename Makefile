rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2)) \
            $(filter $(subst *,%,$2),$(wildcard $1$2))
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
TARGET := ethos.bin
C_FILES := $(call rwildcard,$(SRC_DIR)/,*.c)
ASM_FILES := $(call rwildcard,$(SRC_DIR)/,*.s)
ASM_OBJ := $(patsubst $(SRC_DIR)/%.s, $(OBJ_DIR)/%.o, $(ASM_FILES))
C_OBJ := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(C_FILES))
CFLAGS := -std=gnu99 -ggdb -ffreestanding -O2 -Wall -Wextra -I./src -I./src/libc/include
LDFLAGS := -ffreestanding -ggdb -O2 -nostdlib

image: $(BIN_DIR)/$(TARGET)
	cp $(BIN_DIR)/$(TARGET) isodir/boot/$(TARGET)
	grub-mkrescue -o $(BIN_DIR)/$(TARGET).iso isodir
	mv $(BIN_DIR)/$(TARGET).iso $(BIN_DIR)/ethos.iso

$(BIN_DIR)/$(TARGET): $(C_OBJ) $(ASM_OBJ)
	i686-elf-gcc -Tlinker.ld -o $@ $(LDFLAGS) $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	i686-elf-gcc $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.s
	@mkdir -p $(dir $@)
	nasm -felf32 -o $@ $<


