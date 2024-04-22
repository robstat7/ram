build_and_boot_kernel:
	gcc -I /usr/include/efi -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c boot.c -o build/boot.o -Wall
	
	gcc -I /usr/include/efi -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c main.c -o build/main.o -Wall
	
	gcc -I /usr/include/efi -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c tty_io.c -o build/tty_io.o -Wall
	
	gcc -c printk.c -o build/printk.o -Wall
	
	gcc -c fonts.c -o build/fonts.o -Wall
	
	ld -shared -Bsymbolic -L ../gnu-efi/x86_64/lib/ -L ../gnu-efi/x86_64/gnuefi/ -T /home/dileep/gnu-efi/gnuefi/elf_x86_64_efi.lds \
		../gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o build/boot.o build/main.o build/tty_io.o build/printk.o build/fonts.o -o build/system.so -lefi -lgnuefi
	
	objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 build/system.so build/BOOTx64.EFI
	
	sudo cp build/BOOTx64.EFI /mnt/C96B-D48F/EFI/BOOT/BOOTx64.EFI
	
	reboot
