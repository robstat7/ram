/*
 * Boot loader written using gnu-efi.
 */
#include <efi.h>
#include <efilib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

/* structure to hold frame buffer information */
struct frame_buffer_descriptor {
	long unsigned int *frame_buffer_base;
	unsigned int frame_buffer_size;
	unsigned short horizontal_resolution;
	unsigned short vertical_resolution;
	unsigned short pixels_per_scan_line;
};

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
	struct frame_buffer_descriptor *frame_buffer;

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
			frame_buffer->frame_buffer_base = (long unsigned int *) gop->Mode->FrameBufferBase;
			frame_buffer->frame_buffer_size = gop->Mode->FrameBufferSize;
			frame_buffer->horizontal_resolution = gop->Mode->Info->HorizontalResolution;
			frame_buffer->vertical_resolution = gop->Mode->Info->VerticalResolution;
			frame_buffer->pixels_per_scan_line = gop->Mode->Info->PixelsPerScanLine;
		}
	}

	/* hang here */
	while(1) {
	}

	return 1;
}
