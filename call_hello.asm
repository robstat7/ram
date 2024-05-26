format ELF64

public pre_hello

extrn hello

section '.text' executable

func equ 0x4016d0

pre_hello:
	; push rbp
	; mov rbp, rsp
	; and rsp, 0xfffffffffffffff0
	; call hello
	push r9
	mov r9, func
	call r9
	pop r9
	; mov rsp, rbp
	; pop rbp
	ret
