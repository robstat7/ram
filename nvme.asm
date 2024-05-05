;
; NVMe PCIe driver continued from `nvme.c`
;
format ELF64

public nvme_init_final

section '.text' executable

; registers list
nvme_cc		equ 0x14	; 4-byte controller configuration

; inputs:
; 	rsi = nvme base
; outputs:
;
nvme_init_final:
	; disable the controller if it's enabled
	mov eax, [rsi+nvme_cc]
	btc eax, 0			; clear CC.EN (0) bit to '0'
	jnc nvme_init_disabled		; skip writing to CC if it's already disabled
	mov [rsi+nvme_cc], eax	

nvme_init_disabled:
	ret
