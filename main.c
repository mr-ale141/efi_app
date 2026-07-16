#include <efi.h>
#include <efilib.h>

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  InitializeLib(ImageHandle, SystemTable);
  Print(u"Hello, world from custom kernel!\n");
  for (;;)
  {
    __asm__ volatile("hlt");
  }
  return EFI_SUCCESS;
}
