; Boot Loader written in FASM

format PE64 EFI
entry start

section '.text' code executable readable

start:
	mov [ImageHandle], rcx
	mov [SystemTable], rdx
	; infinite loop
	jmp $

section '.data' code readable writeable

ImageHandle   dq ?
SystemTable   dq ?
