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
NVMe_CSTS	equ 0x1C	; 4-byte Controller Status

; storage memory
nvme_asqb		equ 0x0000000000170000	; 0x170000 -> 0x170FFF	4K admin submission queue base address
nvme_acqb		equ 0x0000000000171000	; 0x171000 -> 0x171FFF	4K admin completion queue base address
os_nvme_iocqb		equ 0x0000000000173000	; 0x173000 -> 0x173FFF	4K I/O Completion Queue Base Address
os_nvme_iosqb		equ 0x0000000000172000	; 0x172000 -> 0x172FFF	4K I/O Submission Queue Base Address
os_nvme_CTRLID		equ 0x0000000000174000	; 0x174000 -> 0x174FFF	4K Controller Identify Data
os_nvme_ANS		equ 0x0000000000175000	; 0x175000 -> 0x175FFF	4K Namespace Data

; Command list
NVMe_ID_NS	equ 0x00	; Identify Namespace data structure for the specified NSID
NVMe_ID_CTRL	equ 0x01	; Identify Controller data structure for the controller
NVMe_ANS	equ 0x02	; Active Namespace ID list

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

nvme_init_enable_wait:
	mov eax, [rsi+NVMe_CSTS]
	bt eax, 1			; CSTS.CFS (1) should be 0. If not the controller has had a fatal error
	jc nvme_init_error
	bt eax, 0			; Wait for CSTS.RDY (0) to become '1'
	jnc nvme_init_enable_wait
	
	; Create I/O Completion Queue
	mov eax, 0x00010005		; CDW0 CID (31:16), PRP used (15:14 clear), FUSE normal (bits 9:8 clear), command Create I/O Completion Queue (0x05)
	xor ebx, ebx			; CDW1 Ignored
	mov ecx, 0x003F0001		; CDW10 QSIZE 64 entries (31:16), QID 1 (15:0)
	mov edx, 0x00000001		; CDW11 PC Enabled (0)
	mov rdi, os_nvme_iocqb		; CDW6-7 DPTR
	call nvme_admin

	; Create I/O Submission Queue
	mov eax, 0x00010001		; CDW0 CID (31:16), PRP used (15:14 clear), FUSE normal (bits 9:8 clear), command Create I/O Submission Queue (0x01)
	xor ebx, ebx			; CDW1 Ignored
	mov ecx, 0x003F0001		; CDW10 QSIZE 64 entries (31:16), QID 1 (15:0)
	mov edx, 0x00010001		; CDW11 CQID 1 (31:16), PC Enabled (0)
	mov rdi, os_nvme_iosqb		; CDW6-7 DPTR
	call nvme_admin

	; Save the Identify Controller structure
	mov eax, 0x00000006		; CDW0 CID 0, PRP used (15:14 clear), FUSE normal (bits 9:8 clear), command Identify (0x06)
	xor ebx, ebx			; CDW1 Ignored
	mov ecx, NVMe_ID_CTRL		; CDW10 CNS
	xor edx, edx			; CDW11 Ignored
	mov rdi, os_nvme_CTRLID		; CDW6-7 DPTR
	call nvme_admin

	; Save the Active Namespace ID list
	mov eax, 0x00000006		; CDW0 CID 0, PRP used (15:14 clear), FUSE normal (bits 9:8 clear), command Identify (0x06)
	xor ebx, ebx			; CDW1 Ignored
	mov ecx, NVMe_ANS		; CDW10 CNS
	xor edx, edx			; CDW11 Ignored
	mov rdi, os_nvme_ANS		; CDW6-7 DPTR
	call nvme_admin

nvme_admin:
	ret

nvme_init_error:
	ret
