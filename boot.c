#include <efi.h>
#include <efilib.h> 

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  InitializeLib(ImageHandle, SystemTable);
  Print(L"Hello, world!\n");
  func();
  return EFI_OUT_OF_RESOURCES;
 // return EFI_SUCCESS;
}
