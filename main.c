#include <efi.h>
#include <efilib.h>

extern EFI_GUID GraphicsOutputProtocol;

static void
fill_boxes(UINT32 *PixelBuffer, UINT32 Width, UINT32 Height)
{
  UINT32 y, x = 0;
  /*
   * This assums BGRR, but it doesn't really matter; we pick red and
   * green so it'll just be blue/green if the pixel format is backwards.
   */
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL Red = {0, 0, 0xff, 0},
  Green = {0, 0xff, 0, 0},
  *Color = NULL;

  for (y = 0; y < Height; y++) {
    Color = ((y / 32) % 2 == 0) ? &Red : &Green;
    for (x = 0; x < Width; x++) {
      if (x % 32 == 0 && x != 0)
        Color = (Color == &Red) ? &Green : &Red;
      PixelBuffer[y * Width + x] = *(UINT32 *)Color;
    }
  }
}

static void
draw_boxes(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop)
{
  int i, imax;
  EFI_STATUS rc;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
  UINTN NumPixels;
  UINT32 *PixelBuffer;
  UINT32 BufferSize;

  if (gop->Mode) {
    imax = gop->Mode->MaxMode;
  } else {
    Print(u"gop->Mode is NULL\n");
    return;
  }

  for (i = 0; i < imax; i++) {
    UINTN SizeOfInfo;
    rc = uefi_call_wrapper(gop->QueryMode, 4, gop, i, &SizeOfInfo,
                           &info);
    if (rc == EFI_NOT_STARTED) {
      Print(u"gop->QueryMode() returned %r\n", rc);
      Print(u"Trying to start GOP with SetMode().\n");
      rc = uefi_call_wrapper(gop->SetMode, 2, gop,
                             gop->Mode ? gop->Mode->Mode : 0);
      rc = uefi_call_wrapper(gop->QueryMode, 4, gop, i,
                             &SizeOfInfo, &info);
    }

    if (EFI_ERROR(rc)) {
      Print(u"%d: Bad response from QueryMode: %r (%d)\n",
            i, rc, rc);
      continue;
    }

    if (CompareMem(info, gop->Mode->Info, sizeof (*info)))
      continue;

    NumPixels = (UINTN)info->VerticalResolution
    * (UINTN)info->HorizontalResolution;
    BufferSize = (UINT32)(NumPixels * sizeof(UINT32));

    PixelBuffer = AllocatePool(BufferSize);
    if (!PixelBuffer) {
      Print(u"Allocation of 0x%08lx bytes failed.\n",
            sizeof(UINT32) * NumPixels);
      return;
    }

    fill_boxes(PixelBuffer,
               info->HorizontalResolution, info->VerticalResolution);

    uefi_call_wrapper(gop->Blt, 10, gop,
                      (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)PixelBuffer,
                      EfiBltBufferToVideo,
                      0, 0, 0, 0,
                      info->HorizontalResolution,
                      info->VerticalResolution,
                      0);
    FreePool(PixelBuffer);
    return;
  }
  Print(u"Never found the active video mode?\n");
}

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{

  EFI_INPUT_KEY efi_input_key;
  EFI_STATUS efi_status = EFI_SUCCESS;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
  EFI_STATUS gop_rc;
  BOOLEAN is_gop_present = TRUE;

  gop_rc = LibLocateProtocol(&GraphicsOutputProtocol, (void **)&gop);
  if (EFI_ERROR(gop_rc)) {
    Print(u"Could not locate GOP: %r\n", gop_rc);
    is_gop_present = FALSE;
  }

  if (!gop) {
    Print(u"LocateProtocol(GOP, &gop) returned %r but GOP is NULL\n", gop_rc);
    is_gop_present = FALSE;
  }

  efi_input_key.UnicodeChar = 0;

  InitializeLib(ImageHandle, SystemTable);
  uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, u"Hello, from custom EFI app!\r\n");
  if (is_gop_present)
  {
    uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, u"Press 'd' to draw\r\n");
  }
  uefi_call_wrapper(SystemTable->ConOut->OutputString, 2, SystemTable->ConOut, u"Press 'x' to exit\r\n");
  while (efi_status == EFI_SUCCESS && efi_input_key.UnicodeChar != 'x')
  {
    WaitForSingleEvent(ST->ConIn->WaitForKey, 0);
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, u"\n\n");
    efi_status = uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &efi_input_key);
    Print(u"ScanCode: %xh  UnicodeChar: %xh CallRtStatus: %x\n",
          efi_input_key.ScanCode, efi_input_key.UnicodeChar, efi_status);

    if (is_gop_present && efi_input_key.UnicodeChar == 'd')
    {
      draw_boxes(gop);
    }
  }

  return EFI_SUCCESS;
}
