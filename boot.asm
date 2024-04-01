; Boot Loader written in FASM

struc UINT32 {
	align 4
	. dd ?
}

struc void {
	align 8
    	. dq ?
}

macro struct name {
	virtual at 0
		name name
	end virtual
}

struc EFI_TABLE_HEADER {
	.Signature		void
    	.Revision		UINT32
    	.HeaderSize             UINT32
    	.CRC32                  UINT32
    	.Reserved               UINT32
}
struct EFI_TABLE_HEADER

struc EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
	.Reset                  void
    	.OutputString           void
    	.TestString             void
    	.QueryMode              void
    	.SetMode                void
    	.SetAttribute           void
    	.ClearScreen            void
    	.SetCursorPosition      void
    	.EnableCursor           void
    	.Mode                   void
}
struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL

struc EFI_SYSTEM_TABLE {
	.Hdr                    EFI_TABLE_HEADER
    	.FirmwareVendor         void
    	.FirmwareRevision       UINT32
    	.ConsoleInHandle        void
    	.ConIn                  void
    	.ConsoleOutHandle       void
    	.ConOut                 void
    	.StandardErrorHandle    void
    	.StdErr                 void
    	.RuntimeServices        void
    	.BootServices           void
    	.NumberOfTableEntries   void
    	.ConfigurationTable     void
}
struct EFI_SYSTEM_TABLE

format PE64 EFI

entry start

section '.text' code executable readable

start:
	; store the image handle and the system table passed by the firmware
	mov [ImageHandle], rcx
	mov [SystemTable], rdx
	
	; set up the GDT
	cli
	lgdt [gdt64.pointer]
	sti

	; hang here
	jmp $

section '.rodata' data readable

; GDT
gdt64:
	dq 0 ; the zero entry
; Code segment. Set the following bits to 1:
; 44: ‘descriptor type’: This has to be 1 for code and data segments
; 47: ‘present’: This is set to 1 if the entry is valid
; 41: ‘read/write’: If this is a code segment, 1 means that it’s readable
; 43: ‘executable’: Set to 1 for code segments
; 53: ‘64-bit’: if this is a 64-bit GDT, this should be set
.code equ $ - gdt64
	dq (1 shl 44) or (1 shl 47) or (1 shl 41) or (1 shl 43) or (1 shl 53)
; Data segment
.data equ $ - gdt64
	dq (1 shl 44) or (1 shl 47) or (1 shl 41)
.pointer:
	dw .pointer - gdt64 - 1 ; length of GDT
	dq gdt64 ; address of GDT

section '.data' data readable writeable

ImageHandle   dq ?
SystemTable   dq ?
