format ELF64

public pre_bash

extrn bash

section '.text' executable

pre_bash:
	push rbp
	mov rbp, rsp
	and rsp, 0xfffffffffffffff0
	call bash
	mov rsp, rbp
	pop rbp
	ret

