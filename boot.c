/*
 * Boot loader written using gnu-efi.
 */
#include <efi.h>
#include <efilib.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "include/frame_buffer.h"

void fill_tty_bgcolor(unsigned short horizontal_resolution, unsigned short vertical_resolution, long unsigned int *frame_buffer_base, long unsigned int bg_color);

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_STATUS status;
	EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
	UINTN SizeOfInfo, numModes, nativeMode;
	int i;
	uint32_t mode;
	struct frame_buffer_descriptor frame_buffer;
	uint8_t mmap[4900];
    	UINTN msize = sizeof(mmap);
	UINTN mkey = 0;
	UINTN dsize = 0;
	UINT32 dversion;


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
  		nativeMode = gop->Mode->Mode;
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


	/* Print(L"fb_base=%p\n", (void *) frame_buffer.frame_buffer_base); */
	
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


	/* fill terminal background color with white */
	fill_tty_bgcolor(frame_buffer.horizontal_resolution, frame_buffer.vertical_resolution, frame_buffer.frame_buffer_base, 0xffffff);

	/* initialize terminal driver */
	tty_init(frame_buffer);

	/* test: write characters onto the terminal */
	/* write_char('B', 0, 0, 0x00000, 0xffffff); */

	write_str("Hello World! Raam Raam sa! The development is going on...", 0, 0, 0x00000, 0xffffff);

end:
	/* hang here */
	while(1) {
	}

	return 1;
}

void fill_tty_bgcolor(unsigned short horizontal_resolution, unsigned short vertical_resolution, long unsigned int *frame_buffer_base, long unsigned int bg_color)
{

	unsigned int pixels = horizontal_resolution * vertical_resolution;
	uint32_t* addr = frame_buffer_base;

	while (pixels--) {
		*addr++ = bg_color;
	}
}
