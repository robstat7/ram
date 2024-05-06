;
; NVMe PCIe driver continued from `nvme.c`
;
format ELF64

extrn nvme_base

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
os_nvme_NSID		equ 0x0000000000176000	; 0x176000 -> 0x176FFF	4K Namespace Identify Data

; Command list
NVMe_ID_NS	equ 0x00	; Identify Namespace data structure for the specified NSID
NVMe_ID_CTRL	equ 0x01	; Identify Controller data structure for the controller
NVMe_ANS	equ 0x02	; Active Namespace ID list

; Memory addresses
os_SystemVariables	equ 0x0000000000110000	; 0x110000 -> System Variables

; DD - Starting at offset 256, increments by 4
os_NVMeTotalLBA		equ os_SystemVariables + 0x010C

; DB - Starting at offset 768, increments by 1
os_NVMeLBA		equ os_SystemVariables + 0x0310
os_NVMe_atail		equ os_SystemVariables + 0x0311

; DW - Starting at offset 512, increments by 2
os_StorageVar		equ os_SystemVariables + 0x0208	; Bit 0 for NVMe

; DQ - Starting at offset 0, increments by 8
os_storage_io		equ os_SystemVariables + 0x00B0

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

	; Save the Identify Namespace data
	mov eax, 0x00000006		; CDW0 CID 0, PRP used (15:14 clear), FUSE normal (bits 9:8 clear), command Identify (0x06)
	mov ebx, 1			; CDW1 NSID
	mov ecx, NVMe_ID_NS		; CDW10 CNS
	xor edx, edx			; CDW11 Ignored
	mov rdi, os_nvme_NSID		; CDW6-7 DPTR
	call nvme_admin

	; Parse the Controller Identify data
	; Serial Number (SN) bytes 23-04
	; Model Number (MN) bytes 63-24
	; Firmware Revision (FR) bytes 71-64
	; Maximum Data Transfer Size (MDTS) byte 77
	; Controller ID (CNTLID) bytes 79-78
	mov rsi, os_nvme_CTRLID
	add rsi, 77
	lodsb
	; The value is in units of the minimum memory page size (CAP.MPSMIN) and is reported as a power of two (2^n).
	; A value of 0h indicates that there is no maximum data transfer size.
	; NVMe_CAP Memory Page Size Maximum (MPSMAX): bits 55:52 - The maximum memory page size is (2 ^ (12 + MPSMAX))
	; NVMe_CAP Memory Page Size Minimum (MPSMIN): bits 51:48 - The minimum memory page size is (2 ^ (12 + MPSMIN))
	; NVMe_CC Memory Page Size (MPS) bits 10:07 - The memory page size is (2 ^ (12 + MPS)). Min 4 KiB, max 128 MiB
	; TODO verify MPS is set within allowed bounds. CC.EN to 0 before changing

	; TODO move this to it's own function
	; Parse the Namespace Identify data for drive 0
	mov rsi, os_nvme_NSID
	lodsd				; Namespace Size (NSZE) bytes 07-00 - Total LBA blocks
	mov r8d, os_NVMeTotalLBA
	; mov [os_NVMeTotalLBA], eax
	mov [r8d], eax

	; Number of LBA Formats (NLBAF) byte 25
	; 0 means only one format is supported. Located at bytes 131:128
	; LBA Data Size (LBADS) is bits 23:16. Needs to be 9 or greater
	; 9 = 512 byte sectors
	; 12 = 4096 byte sectors

	mov r8d, os_nvme_NSID+24	
	mov ecx, [r8d]
	shr ecx, 16
	add cl, 1			; NLBAF is a 0-based number
	mov rsi, os_nvme_NSID+0x80
	xor ebx, ebx
nvme_init_LBA_next:
	cmp cl, 0			; Check # of formats
	je nvme_init_LBA_end
	lodsd				; RP (25:24), LBADS (23:16), MS (15:00)
	shr eax, 16			; AL holds the LBADS
	mov bl, al			; BL holds the highest LBADS so far
	cmp al, bl
	jle nvme_init_LBA_skip
	mov bl, al			; BL holds the highest LBADS so far
nvme_init_LBA_skip:
	dec cl
	jmp nvme_init_LBA_next
nvme_init_LBA_end:
	mov r8d, os_NVMeLBA
	mov [r8d], bl		; Store the highest LBADS

nvme_init_done:
	mov r8d, os_StorageVar
	bts word [r8d], 0	; Set the bit flag that NVMe has been initialized
	mov rdi, os_storage_io
	mov rax, nvme_io
	stosq
	mov rax, nvme_id
	stosq
	ret

nvme_init_error:
	ret

; nvme_admin -- Perform an Admin operation on a NVMe controller
; IN:	EAX = CDW0
;	EBX = CDW1
;	ECX = CDW10
;	EDX = CDW11
;	RDI = CDW6-7
; OUT:	Nothing
;	All other registers preserved
nvme_admin:
	push r9
	push rdi
	push rdx
	push rcx
	push rbx
	push rax
	
	mov r9, rdi			; Save the memory location

	; Build the command at the expected location in the Submission ring
	push rax
	mov rdi, nvme_asqb
	xor eax, eax
	push rcx
	mov rcx, os_NVMe_atail
	mov al, [rcx]		; Get the current Admin tail value
	pop rcx
	shl eax, 6			; Quick multiply by 64
	add rdi, rax
	pop rax

	; Build the structure
	stosd				; CDW0
	mov eax, ebx
	stosd				; CDW1
	xor eax, eax
	stosd				; CDW2
	stosd				; CDW3
	stosq				; CDW4-5
	mov rax, r9
	stosq				; CDW6-7
	xor eax, eax
	stosq				; CDW8-9
	mov eax, ecx
	stosd				; CDW10
	mov eax, edx
	stosd				; CDW11
	xor eax, eax
	stosd				; CDW12
	stosd				; CDW13
	stosd				; CDW14
	stosd				; CDW15

	; Start the Admin command by updating the tail doorbell
	mov rdi, [nvme_base]
	xor eax, eax
	push rcx
	mov rcx, os_NVMe_atail
	mov al, [rcx]		; Get the current Admin tail value
	pop rcx
	mov ecx, eax			; Save the old Admin tail value for reading from the completion ring
	add al, 1			; Add 1 to it
	cmp al, 64			; Is it 64 or greater?
	jl nvme_admin_savetail
	xor eax, eax			; Is so, wrap around to 0
nvme_admin_savetail:
	push rcx
	mov rcx, os_NVMe_atail
	mov [rcx], al		; Save the tail for the next command
	pop rcx
	mov [rdi+0x1000], eax		; Write the new tail value

	; Check completion queue
	mov rdi, nvme_acqb
	shl rcx, 4			; Each entry is 16 bytes
	add rcx, 8			; Add 8 for DW3
	add rdi, rcx
nvme_admin_wait:
	mov rax, [rdi]
	cmp rax, 0x0
	; je nvme_admin_wait		; TODO: fix infinite loop
	xor eax, eax
	stosq				; Overwrite the old entry

	pop rax
	pop rbx
	pop rcx
	pop rdx
	pop rdi
	pop r9
	ret	

	
nvme_io:
	ret


;
; nvme_id -- identify a NVMe device
; inputs:
;	rbx = namespace id
;	rdi = memory location to store data
; outputs:
;	nothing
;	all other registers preserved
;
nvme_id:
	ret
