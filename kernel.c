#include <efi.h>
#include <efilib.h> 
#include <string.h>
#include <stdio.h>

UINT64 FileSize(EFI_FILE_HANDLE FileHandle);

// typedef struct {
// 	uint16_t	limit;
// 	uint64_t	base;
// } __attribute__((packed)) idtr_t;
// 
// idtr_t *efi_idtr;

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_LOADED_IMAGE *loaded_image;                  /* image interface */
  	EFI_GUID lipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;      /* image interface GUID */
  	EFI_FILE_HANDLE Volume;   
	CHAR16              *FileName;
  	EFI_FILE_HANDLE     FileHandle;
	UINT64              ReadSize;
  	UINT8               *Buffer;

	loaded_image = NULL;
	FileName = L"bash";

  	InitializeLib(ImageHandle, SystemTable);

	/* set up the GDT */
	// gdt_install();	
	
	/* load bash program 4 loadable segments in memory */
	/* 1. read file into memory first */
	uefi_call_wrapper(BS->HandleProtocol, 3, ImageHandle, &lipGuid, (void **) &loaded_image);
	/* get the volume handle */
	Volume = LibOpenRoot(loaded_image->DeviceHandle);

	/* open the file */
  	uefi_call_wrapper(Volume->Open, 5, Volume, &FileHandle, FileName, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
	
	/* read file into memory */
	ReadSize = FileSize(FileHandle);
	Buffer = AllocatePool(ReadSize);

	uefi_call_wrapper(FileHandle->Read, 3, FileHandle, &ReadSize, Buffer);

	Print(L"loading segs...\n");
	/* load loadable segments */
	memcpy((void *)0x0000000000400000, (void *)Buffer, (size_t)0x0000000000000720);	
	memcpy((void *)0x0000000000401000, (void *)(Buffer + 0x0000000000001000), (size_t)0x000000000013e9a1);	
	memcpy((void *)0x0000000000540000, (void *)(Buffer + 0x0000000000140000), (size_t)0x0000000000045e1f);	
	memcpy((void *)0x0000000000586978, (void *)(Buffer + 0x0000000000186978), (size_t)0x0000000000008f60);	


	/* store idt register to variable efi_idtr */
	// asm volatile ("sidt %0" : "=m"(*efi_idtr));

	/* init idt */
	idt_init();

	// Print(L"making syscall #2 ...\n");
        // __asm__ volatile ("movq $2, %rax"); 
        // __asm__ volatile ("syscall");

	/* entry point of bash */

	void (*bash)(void) = (void (*)())0x4033e0;
	Print(L"executing bash ...\n");
	bash();

	Print(L"I should not be printed!...\n");

  	FreePool(loaded_image);
  	FreePool(FileName);
  	FreePool(Buffer);

	/* hang here */
	for(;;) {
	}

  	return EFI_OUT_OF_RESOURCES;
}

UINT64 FileSize(EFI_FILE_HANDLE FileHandle)
{
  UINT64 ret;
  EFI_FILE_INFO       *FileInfo;         /* file information structure */
  /* get the file's size */
  FileInfo = LibFileInfo(FileHandle);
  ret = FileInfo->FileSize;
  FreePool(FileInfo);
  return ret;
}
