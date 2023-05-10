ASM := nasm
CC := x86_64-elf-gcc
LD := x86_64-elf-ld

CFLAGS := -std=gnu99 -ffreestanding -O2 -Wall -Wextra

PROJECT := NOS

# list of all C source files
kernel_source_files := $(shell find proj/src/kernel -name *.c)
kernel_object_files := $(patsubst proj/src/kernel/%.c, build/obj/kernel/%.o, $(kernel_source_files))

x86_64_c_source_files := $(shell find proj/src/x86_64 -name *.c)
x86_64_c_object_files := $(patsubst proj/src/x86_64/%.c, build/obj/x86_64/%.o, $(x86_64_c_source_files))

# list of all assembly source files
x86_64_asm_source_files := $(shell find proj/src/boot -name *.asm)
x86_64_asm_object_files := $(patsubst proj/src/boot/%.asm, build/obj/boot/%.o, $(x86_64_asm_source_files))

x86_64_object_files := $(x86_64_c_object_files) $(x86_64_asm_object_files)

all: $(PROJECT).bin verify

run: all iso emu-iso 

$(kernel_object_files): build/obj/kernel/%.o : proj/src/kernel/%.c
	@mkdir -p $(dir $@) && \
	$(CC) -c -I proj/include $(CFLAGS) $(patsubst build/obj/kernel/%.o, proj/src/kernel/%.c, $@) -o $@
	
$(x86_64_c_object_files): build/obj/x86_64/%.o : proj/src/x86_64/%.c
	@mkdir -p $(dir $@) && \
	$(CC) -c -I proj/include $(CFLAGS) $(patsubst build/obj/x86_64/%.o, proj/src/x86_64/%.c, $@) -o $@
	
$(x86_64_asm_object_files): build/obj/boot/%.o : proj/src/boot/%.asm
	@mkdir -p $(dir $@) && \
	$(ASM) -felf64 $(patsubst build/obj/boot/%.o, proj/src/boot/%.asm, $@) -o $@

$(PROJECT).bin: $(kernel_object_files) $(x86_64_object_files)
	@mkdir -p dist/ && \
	$(LD) -n -T proj/linker.ld -o dist/$(PROJECT).bin $(kernel_object_files) $(x86_64_object_files)

verify:
	@if grub-file --is-x86-multiboot2 dist/$(PROJECT).bin; then \
			echo multiboot2 confirmed; \
	else \
			echo the file is not multiboot2; \
	fi

iso: $(PROJECT).bin
	@mkdir -p dist/iso/boot/grub

	@cp dist/$(PROJECT).bin dist/iso/boot/$(PROJECT).bin
	@cp proj/grub.cfg dist/iso/boot/grub/grub.cfg
	@grub-mkrescue -o dist/$(PROJECT).iso dist/iso


# qemu-system-x86_64 -kernel dist/Builder.bin
emu-bin:
	qemu-system-x86_64 -kernel dist/$(PROJECT).bin
# qemu-system-x86_64 -cdrom dist/Builder.iso
emu-iso:
	qemu-system-x86_64 -cdrom dist/$(PROJECT).iso

clean:
	-@rm -rf build dist

.PHONY: all


