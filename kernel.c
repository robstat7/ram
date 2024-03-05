#include <efi.h>
#include <efilib.h> 

UINT64 FileSize(EFI_FILE_HANDLE FileHandle);

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

	/* load bash program 4 loadable segments in memory */
	/* 1. read file into memory first */
	uefi_call_wrapper(BS->HandleProtocol, 3, ImageHandle, &lipGuid, (void **) &loaded_image);
	/* get the volume handle */
	Volume = LibOpenRoot(loaded_image->DeviceHandle);

	/* open the file */
  	uefi_call_wrapper(Volume->Open, 5, Volume, &FileHandle, FileName, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
	
	ReadSize = FileSize(FileHandle);
	Buffer = AllocatePool(ReadSize);

	uefi_call_wrapper(FileHandle->Read, 3, FileHandle, &ReadSize, Buffer);


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
