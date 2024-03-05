; Boot Loader written in FASM

struc UINT32
{
    align 4
    . dd ?
}

struc void
{
    align 8
    . dq ?
}

macro struct name
{
    virtual at 0
      name name
    end virtual
}

struc EFI_TABLE_HEADER
{
    .Signature              void
    .Revision               UINT32
    .HeaderSize             UINT32
    .CRC32                  UINT32
    .Reserved               UINT32
}
struct EFI_TABLE_HEADER

struc EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL
{
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

struc EFI_SYSTEM_TABLE
{
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

SYSSEG		= 0x1000		; load address of ram

STACK_SIZE	= 1024			; minimum number of bytes for stack

entry start

; This section contains data initialised to zeroes when the kernel is loaded.
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

	; write message
	enter 32, 0		; align stack and create shadow space for local
				; calls

	mov rdx, 1
	mov rcx, [SystemTable]
	mov rcx, [rcx + EFI_SYSTEM_TABLE.ConOut]
	call [rcx + EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.Reset]

	mov rdx, 0x0f		; EFI_WHITE
	mov rcx, [SystemTable]
	mov rcx, [rcx + EFI_SYSTEM_TABLE.ConOut]
	call [rcx + EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.SetAttribute]

	lea rdx, [msg]
	mov rcx, [SystemTable]
	mov rcx, [rcx + EFI_SYSTEM_TABLE.ConOut]
	call [rcx + EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString]

	leave			; restore stack

	; now load ram at 0x1000


	; infinite loop
	jmp $

section '.data' data readable writeable

ImageHandle   dq ?
SystemTable   dq ?
msg 	      du "Loading system ...", 10, 0
