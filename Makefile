#
# Makefile for my_kernel.
#

BOOTx64: boot/boot.s
	fasm boot/boot.s build/BOOTx64.EFI
clean:
	rm -r build/
