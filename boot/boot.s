; Boot Loader written in FASM

format PE64 EFI

SYSSEG		= 0x1000		; load address of ram

STACK_SIZE	= 1024			; minimum number of bytes for stack

entry start

; this section contains data initialised to zeroes when the kernel is loaded.
section '.bss' data readable writeable

; our C code will need a stack to run. Here, we allocate 1024 bytes
; (or 1 Kilobytes) for our stack.
stack_bottom:
	db STACK_SIZE dup (0)
stack_top:

section '.text' code executable readable

start:
	mov [ImageHandle], rcx
	mov [SystemTable], rdx

				; First thing's first: we want to set up an
				; environment that's ready to run C code. C is
				; very relaxed in its requirements: All we need
				; to do is to set up the stack. Please note that
				; on x86-64, the stack grows DOWNWARD. This is
				; why we start at the top.

	mov [stack_top], rsp	; set the stack pointer to the top of the stack

	; load ram at 0x1000

	; infinite loop
	jmp $

section '.data' data readable writeable

ImageHandle   dq ?
SystemTable   dq ?
