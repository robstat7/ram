#
# Makefile for ram.
#

all: build/BOOTx64.EFI build/kernel.o

build/BOOTx64.EFI: boot/boot.s
	fasm $< $@
build/kernel.o: kernel/kernel.c
	gcc -c $< -o $@
clean:
	rm -f build/BOOTx64.EFI build/kernel.o
