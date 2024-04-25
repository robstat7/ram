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
#include "include/frame_buffer.h"

int8_t validate_xsdp_checksum(void *table);

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
	uint8_t mmap[4900];
    	UINTN msize = sizeof(mmap);
	UINTN mkey = 0;
	UINTN dsize = 0;
	int num_config_tables;
	EFI_CONFIGURATION_TABLE *config_tables;
	EFI_GUID Acpi20TableGuid = ACPI_20_TABLE_GUID;	/* EFI GUID for a pointer to the ACPI 2.0 or later specification XSDP structure */
	void *table;
	void *xsdp;
	uint8_t revision;

	xsdp = NULL;
	table = NULL;
	

	InitializeLib(ImageHandle, SystemTable);

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
	while(1) {
	}

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
