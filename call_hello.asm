format ELF64

public pre_hello

extrn hello

section '.text' executable

pre_hello:
	; push rbp
	; mov rbp, rsp
	; and rsp, 0xfffffffffffffff0
	call hello
	; mov rsp, rbp
	; pop rbp
	ret
