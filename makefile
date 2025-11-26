# -----------------------------
# Makefile for simple OS
# -----------------------------

# Tools
NASM    := nasm
CC      := i686-elf-gcc
LD      := ld
OBJCOPY := objcopy

CFLAGS  := -m32 -ffreestanding -O1 -Iinclude -pedantic -isystem /usr/include -Wno-cast-function-type
LDFLAGS := -m elf_i386 -T linker.ld

# Sources
BOOT_SRC   := src/boot/bootloader.s
ENTRY_SRC  := src/boot/entry.s
KERNEL_SRC := $(shell find src/kernel/ -name '*.c')

# Objects
OBJ_ENTRY  := $(ENTRY_SRC:.s=.o)
OBJ_KERNEL := $(KERNEL_SRC:.c=.o)
OBJ_ALL    := $(OBJ_ENTRY) $(OBJ_KERNEL)

# Binaries
KERNEL_ELF     := kernel.elf
KERNEL_BIN     := kernel.bin
BOOTLOADER_BIN := bootloader.bin
BOOTABLE_BIN   := os.img

# -----------------------------
# Default target
# -----------------------------
all: clean bootable

# -----------------------------
# Compile entry + kernel
# -----------------------------
%.o: %.s
	$(NASM) -f elf32 $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 2>>build.log

# -----------------------------
# Link kernel ELF
# -----------------------------
kernel: $(OBJ_ALL)
	$(LD) $(LDFLAGS) -o $(KERNEL_ELF) $^ 2>>build.log

# Convert ELF -> flat binary
$(KERNEL_BIN): kernel
	$(OBJCOPY) -O binary $(KERNEL_ELF) $(KERNEL_BIN)
	@size=$$(stat -c%s "$(KERNEL_BIN)"); \
	pad=$$(( (512 - (size % 512)) % 512 )); \
	if [ $$pad -ne 0 ]; then \
		truncate -s $$(($$size + $$pad)) $(KERNEL_BIN); \
	fi

# -----------------------------
# Compile bootloader
# -----------------------------
bootloader: $(BOOT_SRC) $(KERNEL_BIN)
	@size=$$(stat -c%s "$(KERNEL_BIN)"); \
	sectors=$$((size / 512)); \
	echo "Kernel size: $$sectors sectors"; \
	$(NASM) -f bin -DKERNEL_SECTORS=$$sectors $< -o $(BOOTLOADER_BIN)
	@echo "Bootloader binary created: $(BOOTLOADER_BIN)"

# -----------------------------
# Create bootable OS
# -----------------------------
bootable: clean kernel bootloader
	@cat $(BOOTLOADER_BIN) $(KERNEL_BIN) > $(BOOTABLE_BIN)
	@truncate -s 128M tmp.bin
	@echo "Bootable OS created: $(BOOTABLE_BIN)"
	@rm -f $(OBJ_ALL) *.elf *.bin

# -----------------------------
# Run in QEMU
# -----------------------------
run: bootable
	qemu-system-x86_64 $(BOOTABLE_BIN) -m 4G -enable-kvm -cpu host -smp 8

# -----------------------------
# Clean
# -----------------------------
clean:
	rm -f $(OBJ_ALL) $(KERNEL_ELF) $(KERNEL_BIN) $(BOOTLOADER_BIN) $(BOOTABLE_BIN) build.log *.bin

.PHONY: all kernel bootloader bootable clean run
