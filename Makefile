ibuild_and_boot_kernel:
	gcc -masm=intel -I /usr/include/efi -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c kernel.c -o build/kernel.o -Wall
	
	# gcc -I /usr/include/efi -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -mgeneral-regs-only -c interrupt_handler.c -o interrupt_handler.o
	
	# gcc -I /usr/include/efi -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c idt.c -o idt.o
	
	# gcc -c isr.S -o isr.o
	
	gcc -I /usr/include/efi -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -mcmodel=large -mno-mmx -mno-sse -mno-sse2 -c isr.c -o build/isr.o -Wall
	
	nasm descriptor_load.asm -f elf64 -o build/descriptor_load.o
	
	nasm interrupt.asm -f elf64 -o build/interrupt.o
	
	fasm call_bash.asm build/call_bash.o
	
	ld -shared -Bsymbolic -L ../gnu-efi/x86_64/lib/ -L ../gnu-efi/x86_64/gnuefi/ -T /home/dileep/gnu-efi/gnuefi/elf_x86_64_efi.lds \
		../gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o build/kernel.o build/call_bash.o build/isr.o build/descriptor_load.o build/interrupt.o -o build/system.so -lefi -lgnuefi
	
	objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 build/system.so build/BOOTx64.EFI
	
	sudo cp build/BOOTx64.EFI /mnt/C96B-D48F/EFI/BOOT/BOOTx64.EFI
	
	reboot
