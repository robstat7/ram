;
; NVMe PCIe driver continued from `nvme.c`
;
format ELF64

public nvme_init_final

section '.text' executable

; registers list
nvme_cc		equ 0x14	; 4-byte controller configuration
nvme_aqa	equ 0x24	; 4-byte Admin Queue Attributes
nvme_asq	equ 0x28	; 8-byte admin submission queue base address
nvme_acq	equ 0x30	; 8-byte admin completion queue base address
nvme_intms	equ 0x0C	; 4-byte interrupt mask set

; storage memory
nvme_asqb		equ 0x0000000000170000	; 0x170000 -> 0x170FFF	4K admin submission queue base address
nvme_acqb		equ 0x0000000000171000	; 0x171000 -> 0x171FFF	4K admin completion queue base address

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
	; configure AQA, ASQ, and ACQ
	mov eax, 0x003F003F		; 64 commands each for ACQS (27:16) and ASQS (11:00)
	mov [rsi+nvme_aqa], eax
	mov rax, nvme_asqb		; ASQB 4K aligned (63:12)
	mov [rsi+nvme_asq], rax
	mov rax, nvme_acqb		; ACQB 4K aligned (63:12)
	mov [rsi+nvme_acq], rax

	; disable controller interrupts
	mov eax, 0xffffffff		; mask all interrupts
	mov [rsi+nvme_intms], eax
	
	; enable the controller
	mov eax, 0x00460001		; set iocqes (23:20), iosqes (19:16), and en (0)
	mov [rsi+nvme_cc], eax		; write the new cc value and enable controller
	ret
