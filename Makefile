BUILD_DIR_NAME=build

all: build_submodule clean build

build_submodule:
	make -C gnu-efi

mkdir:
	mkdir -p $(BUILD_DIR_NAME)
	
main.o: mkdir
	gcc -Ignu-efi/inc -fpic -ffreestanding -fno-stack-protector \
		-fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args \
		-c main.c -o $(BUILD_DIR_NAME)/main.o

main.so: main.o
	ld -shared -Bsymbolic -Lgnu-efi/x86_64/lib -Lgnu-efi/x86_64/gnuefi \
		-Tgnu-efi/gnuefi/elf_x86_64_efi.lds gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o \
		$(BUILD_DIR_NAME)/main.o -o $(BUILD_DIR_NAME)/main.so -lgnuefi -lefi

build: main.so
	objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym  \
		-j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --output-target \
		efi-app-x86_64 --subsystem=10 $(BUILD_DIR_NAME)/main.so $(BUILD_DIR_NAME)/main.efi

clean:
	rm -rf $(BUILD_DIR_NAME)

copy:
	sudo rm -f /mnt/EFI/BOOT/BOOTX64.EFI
	sudo cp build/main.efi /mnt/EFI/BOOT/BOOTX64.EFI

