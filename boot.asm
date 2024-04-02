; Boot Loader written in FASM
struc UINT8
{
    . db ?
}

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

struc EFI_BOOT_SERVICES
{
    .Hdr                                    EFI_TABLE_HEADER                  
    .RaiseTPL                               void  ; EFI_RAISE_TPL                     
    .RestoreTPL                             void  ; EFI_RESTORE_TPL                   
    .AllocatePages                          void  ; EFI_ALLOCATE_PAGES                
    .FreePages                              void  ; EFI_FREE_PAGES                    
    .GetMemoryMap                           void  ; EFI_GET_MEMORY_MAP                
    .AllocatePool                           void  ; EFI_ALLOCATE_POOL                 
    .FreePool                               void  ; EFI_FREE_POOL                     
    .CreateEvent                            void  ; EFI_CREATE_EVENT                  
    .SetTimer                               void  ; EFI_SET_TIMER                     
    .WaitForEvent                           void  ; EFI_WAIT_FOR_EVENT                
    .SignalEvent                            void  ; EFI_SIGNAL_EVENT                  
    .CloseEvent                             void  ; EFI_CLOSE_EVENT                   
    .CheckEvent                             void  ; EFI_CHECK_EVENT                   
    .InstallProtocolInterface               void  ; EFI_INSTALL_PROTOCOL_INTERFACE    
    .ReinstallProtocolInterface             void  ; EFI_REINSTALL_PROTOCOL_INTERFACE  
    .UninstallProtocolInterface             void  ; EFI_UNINSTALL_PROTOCOL_INTERFACE  
    .HandleProtocol                         void  ; EFI_HANDLE_PROTOCOL               
    .Reserved                               void
    .RegisterProtocolNotify                 void  ; EFI_REGISTER_PROTOCOL_NOTIFY               
    .LocateHandle                           void  ; EFI_LOCATE_HANDLE                          
    .LocateDevicePath                       void  ; EFI_LOCATE_DEVICE_PATH                     
    .InstallConfigurationTable              void  ; EFI_INSTALL_CONFIGURATION_TABLE            
    .LoadImage                              void  ; EFI_IMAGE_LOAD                             
    .StartImage                             void  ; EFI_IMAGE_START                            
    .Exit                                   void  ; EFI_EXIT                                   
    .UnloadImage                            void  ; EFI_IMAGE_UNLOAD                           
    .ExitBootServices                       void  ; EFI_EXIT_BOOT_SERVICES                     
    .GetNextMonotonicCount                  void  ; EFI_GET_NEXT_MONOTONIC_COUNT               
    .Stall                                  void  ; EFI_STALL                                  
    .SetWatchdogTimer                       void  ; EFI_SET_WATCHDOG_TIMER                     
    .ConnectController                      void  ; EFI_CONNECT_CONTROLLER                     
    .DisconnectController                   void  ; EFI_DISCONNECT_CONTROLLER                  
    .OpenProtocol                           void  ; EFI_OPEN_PROTOCOL                          
    .CloseProtocol                          void  ; EFI_CLOSE_PROTOCOL                         
    .OpenProtocolInformation                void  ; EFI_OPEN_PROTOCOL_INFORMATION              
    .ProtocolsPerHandle                     void  ; EFI_PROTOCOLS_PER_HANDLE                   
    .LocateHandleBuffer                     void  ; EFI_LOCATE_HANDLE_BUFFER                   
    .LocateProtocol                         void  ; EFI_LOCATE_PROTOCOL                        
    .InstallMultipleProtocolInterfaces      void  ; EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES   
    .UninstallMultipleProtocolInterfaces    void  ; EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES 
    .CalculateCrc32                         void  ; EFI_CALCULATE_CRC32                        
    .CopyMem                                void  ; EFI_COPY_MEM                               
    .SetMem                                 void  ; EFI_SET_MEM                                
    .CreateEventEx                          void  ; EFI_CREATE_EVENT_EX                        
}
struct EFI_BOOT_SERVICES

; Graphics
struc EFI_GRAPHICS_OUTPUT_BLT_PIXEL
{
    .Blue                   UINT8
    .Green                  UINT8
    .Red                    UINT8
    .Reserved               UINT8
}
struct EFI_GRAPHICS_OUTPUT_BLT_PIXEL

struc EFI_PIXEL_BITMASK
{
    .RedMask                UINT32    
    .GreenMask              UINT32    
    .BlueMask               UINT32    
    .ReservedMask           UINT32    
}
struct EFI_PIXEL_BITMASK

struc EFI_GRAPHICS_OUTPUT_MODE_INFORMATION
{
    .Version                UINT32
    .HorizontalResolution   UINT32
    .VerticalResolution     UINT32
    .PixelFormat            UINT32
    .PixelInformation       EFI_PIXEL_BITMASK        
    .PixelsPerScanLine      UINT32
}
struct EFI_GRAPHICS_OUTPUT_MODE_INFORMATION

struc EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE
{
    .MaxMode                UINT32
    .Mode                   UINT32
    .Info                   EFI_GRAPHICS_OUTPUT_MODE_INFORMATION
    .SizeOfInfo             void
    .FrameBufferBase        void
    .FrameBufferSize        void
}
struct EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE

struc EFI_GRAPHICS_OUTPUT_PROTOCOL
{
    .QueryMode              void
    .SetMode                void
    .Blt                    void
    .Mode                   EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE        
}
struct EFI_GRAPHICS_OUTPUT_PROTOCOL

format PE64 EFI

entry start

section '.text' code executable readable

start:
	; store the image handle and the system table passed by the firmware
	mov [ImageHandle], rcx
	mov [SystemTable], rdx

	; detect GOP
	mov rcx, EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID
	mov rdx, 0
    	lea r8, [GOP_Handle]
    	mov r9, [SystemTable]
    	mov r9, [r9 + EFI_SYSTEM_TABLE.BootServices]
    	call [r9 + EFI_BOOT_SERVICES.LocateProtocol]
	
	; store the framebuffer
    	mov rcx, [GOP_Handle]
	mov rdx, [rcx + EFI_GRAPHICS_OUTPUT_PROTOCOL.Mode]
	mov r8, [rdx + EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.FrameBufferBase]
	mov [Frame_Buffer_Base], r8

	mov r8, [rdx + EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.FrameBufferSize]
	mov [Frame_Buffer_Size], r8
	
	; get memory map
	mov rdx, [SystemTable]
 
	sub rsp, 5*8

	mov rax, [rdx+EFI_SYSTEM_TABLE.BootServices]
	mov rax, [rax+EFI_BOOT_SERVICES.GetMemoryMap]
	lea rcx, [MemoryMapSize]
	lea rdx, [MemoryMap]
	lea r8, [MapKey]
	lea r9, [DescriptorSize]
	lea r10, [DescriptorVersion]
	mov qword[rsp + 8*4], r10

	call rax

	add rsp, 5*8
	
	cmp rax, 0 ; EFI_SUCCESS
	jne .Fail
.Success:
	; exit boot services
	mov rdx,[SystemTable]

	mov rax, [rdx+EFI_SYSTEM_TABLE.BootServices]
	mov rax, [rax+EFI_BOOT_SERVICES.ExitBootServices]
	mov rcx, ImageHandle
	mov rdx, [MapKey]

	call rax
	hlt
	
.Fail:
	mov rdx,TestNotOkText
	jmp .End

	; set up the GDT
	cli
	lgdt [gdt64.pointer]
	sti
.End:
	mov rax,[SystemTable]
        
        push rbp
        sub rsp,0x20
        
        mov rcx,[rax+EFI_SYSTEM_TABLE.ConOut]
        mov rcx,[rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString]

	call rcx
	add rsp,0x20
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
MemoryMapSize      dq 1024*1024		; 1 MB should be enough
MemoryMap          rb MemoryMapSize	; buffer for memory map
MapKey             dq ?
DescriptorSize     dq ?
DescriptorVersion  dd ?    
msg 	      du "Loading system ...", 10, 0
TestOkText:           du 'Test OK',0
TestNotOkText:           du 'Test Not OK',0
TestNotOkText2:           du 'Exit boot services failed!',0
EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID    db 0xde, 0xa9, 0x42, 0x90, 0xdc, 0x23, 0x38, 0x4a, 0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a
GOP_Handle            dq    EFI_GRAPHICS_OUTPUT_PROTOCOL
Frame_Buffer_Base     dq ?
Frame_Buffer_Size     dq ?
