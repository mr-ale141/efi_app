#include <efi.h>
#include <efilib.h>

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{

  EFI_INPUT_KEY efi_input_key;
  EFI_STATUS efi_status = EFI_SUCCESS;

  efi_input_key.UnicodeChar = 0;

  InitializeLib(ImageHandle, SystemTable);
  uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, u"Hello, from custom EFI app!\r\n");
  uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, u"Press 'x' to exit\r\n");
  while (efi_status == EFI_SUCCESS && efi_input_key.UnicodeChar != 'x')
  {
    WaitForSingleEvent(ST->ConIn->WaitForKey, 0);
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, u"\n\n");
    efi_status = uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &efi_input_key);
    Print(u"ScanCode: %xh  UnicodeChar: %xh CallRtStatus: %x\n",
          efi_input_key.ScanCode, efi_input_key.UnicodeChar, efi_status);
  }
  return EFI_SUCCESS;
}
