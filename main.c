#include <efi.h>
#include <efilib.h>

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{

  UINTN index;

  InitializeLib(ImageHandle, SystemTable);
  uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, u"Hello, from custom EFI app!\r\n");
  uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, u"\r\n\r\n\r\nHit any key to exit\r\n");
  uefi_call_wrapper(SystemTable->BootServices->WaitForEvent, 3, 1, &SystemTable->ConIn->WaitForKey, &index);
  return EFI_SUCCESS;
}
