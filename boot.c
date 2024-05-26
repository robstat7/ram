/*
 * Raam Raam sa _/\_ _/\_ _/\_
 *
 * Boot loader written using gnu-efi
 */
#include <efi.h>
#include <efilib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "fb.h"

int8_t validate_xsdp_checksum(void *table);
UINT64 FileSize(EFI_FILE_HANDLE FileHandle);

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_STATUS status;
	EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
	UINTN SizeOfInfo, numModes;
	int i;
	uint32_t mode;
	struct frame_buffer_descriptor frame_buffer;
	uint8_t mmap[8900];
    	UINTN msize = sizeof(mmap);
	UINTN mkey = 0;
	UINTN dsize = 0;
	int num_config_tables;
	EFI_CONFIGURATION_TABLE *config_tables;
	EFI_GUID Acpi20TableGuid = ACPI_20_TABLE_GUID;	/* EFI GUID for a pointer to the ACPI 2.0 or later specification XSDP structure */
	void *table;
	void *xsdp;
	uint8_t revision;
	EFI_LOADED_IMAGE *loaded_image;                  /* image interface */
  	EFI_GUID lipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;      /* image interface GUID */
  	EFI_FILE_HANDLE Volume;
	CHAR16              *FileName;
  	EFI_FILE_HANDLE     FileHandle;
	UINT64              ReadSize;
  	UINT8               *Buffer;


	FileName = L"hello";
	loaded_image = NULL;
	xsdp = NULL;
	table = NULL;
	

	InitializeLib(ImageHandle, SystemTable);
	



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
// 	memcpy((void *)0x0000000000400000, (void *)Buffer, (size_t)0x00000000000004f8);
// 	memcpy((void *)0x0000000000401000, (void *)(Buffer + 0x0000000000001000), (size_t)0x000000000007ddf1);
// 	memcpy((void *)0x000000000047f000, (void *)(Buffer + 0x000000000007f000), (size_t)0x0000000000027a8c);
// 	memcpy((void *)0x00000000004a72f0, (void *)(Buffer + 0x00000000000a72f0), (size_t)0x00000000000057b8);


 	memcpy((void *)0x4016d0, (void *)Buffer, (size_t)2);





	/* detecting GOP */
	status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void**)&gop);
	if(EFI_ERROR(status))
		Print(L"error: unable to locate GOP!\n");

	/* get the current mode */
  	status = uefi_call_wrapper(gop->QueryMode, 4, gop, gop->Mode==NULL?0:gop->Mode->Mode, &SizeOfInfo, &info);
  	/* this is needed to get the current video mode */
  	if (status == EFI_NOT_STARTED)
  		status = uefi_call_wrapper(gop->SetMode, 2, gop, 0);
  	if(EFI_ERROR(status)) {
  		Print(L"error: unable to get native video mode!\n");
  	} else {
  		numModes = gop->Mode->MaxMode;
  	}

	/* query available video modes and get the mode number for 1280 x 1024 resolution */
	mode = -1;
	for (i = 0; i < numModes; i++) {
		status = uefi_call_wrapper(gop->QueryMode, 4, gop, i, &SizeOfInfo, &info);
		if (info->HorizontalResolution == 1280 && info->VerticalResolution == 1024 && info->PixelFormat == 1) {
			mode = i;
			break;
		}
  	}

	if (mode == -1) {
		Print(L"error: unable to get video mode for 1280 x 1024 resolution!\n");
	} else {
		/* set video mode and get the framebuffer */
		status = uefi_call_wrapper(gop->SetMode, 2, gop, mode);
		if(EFI_ERROR(status)) {
			Print(L"error: unable to set video mode %03d\n", mode);
		} else {
			/* store framebuffer information */
			frame_buffer.frame_buffer_base = (long unsigned int *) gop->Mode->FrameBufferBase;
			frame_buffer.frame_buffer_size = gop->Mode->FrameBufferSize;
			frame_buffer.horizontal_resolution = gop->Mode->Info->HorizontalResolution;
			frame_buffer.vertical_resolution = gop->Mode->Info->VerticalResolution;
			frame_buffer.pixels_per_scan_line = gop->Mode->Info->PixelsPerScanLine;
		}
	}


	/* locating and storing the pointer to the XSDP structure */	
	num_config_tables = SystemTable->NumberOfTableEntries;

	config_tables = SystemTable->ConfigurationTable;	

	for(i = 0; i < num_config_tables; i++) {
		if (CompareGuid(&config_tables[i].VendorGuid, &Acpi20TableGuid) == 0) {
			table = config_tables[i].VendorTable;
			/* validate the XSDP */
			/* check if ACPI version is >= 2.0 */
			revision = *((uint8_t *) table + 15);
			if(revision == 0x2) {
				/* validate checksum */
				if(validate_xsdp_checksum(table) == 0) {
					/* store xsdp struct pointer */
					xsdp = table;
					break;
				}
			}
		}
	}

	if(xsdp == NULL) {
		Print(L"error: could not find XSDP structure pointer!\n");
		goto end;
	}

	/* free resources */
	// FreePool(loaded_image);
  	FreePool(FileName);
  	FreePool(Buffer);

	
	/* try to exit boot services 3 times */
  	for (i = 0; i < 3; i++) {
		/* get memory map */
		status = uefi_call_wrapper(BS->GetMemoryMap, 5, &msize, &mmap, &mkey, &dsize, NULL);
		if(EFI_ERROR(status)) {
			Print(L"error: could not get memory map!\n");
			goto end;
		} else if (status == EFI_SUCCESS) {
			/* exit boot services */
			status = uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, mkey);
			if (status == EFI_SUCCESS)
				break;
		}
	}
	if(status == EFI_INVALID_PARAMETER) {
		Print(L"error: exit boot services: map key is incorrect!\n");
		goto end;
	}


	/* jump to kernel */
	main(frame_buffer, xsdp);


	/* should not reach here */
end:
	/* hang here */
	for(;;);

	return 1;
}

/*
 * returns 0 if the checksum is valid.
 */
int8_t validate_xsdp_checksum(void *table)
{
	uint8_t rsdp_start_offset;
	uint8_t xsdp_start_offset;
	uint8_t rsdp_end_offset;
	uint8_t xsdp_end_offset;
	int sum;
	uint8_t i;

	rsdp_start_offset = 0;
	rsdp_end_offset= 19;
	xsdp_start_offset = 20;
	xsdp_end_offset = 35;
	sum = 0;

	for(i = rsdp_start_offset; i <= rsdp_end_offset; i++)
		sum += *((int8_t *) table + i);

	if((int8_t) sum == 0) {
		for(i = xsdp_start_offset; i <= xsdp_end_offset; i++)
			sum += *((int8_t *) table + i);
	}

	return (int8_t) sum;
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
