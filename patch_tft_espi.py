"""Pre-build patch for the vendored TFT_eSPI submodule.

WHY THIS EXISTS
---------------
lib/TFT_eSPI is a git submodule, so the parent repo only records a commit
pointer - never file contents. A local edit inside it is invisible to this
repo's history and is wiped by any `git submodule update` or fresh clone.
This script keeps the patch IN this repo and re-applies it on every build, so
it travels with the project.

WHAT IT PATCHES
---------------
TFT_eSPI::pushImageDMA(..., uint16_t* image, uint16_t* buffer) ends with:

    ret = spi_device_queue_trans(dmaHAL, &trans, portMAX_DELAY);
    assert(ret == ESP_OK);

On a failed DMA queue that assert() calls abort() and REBOOTS the board, and
throws away the error code that would explain why. That cost us a run of
random reboots during full-auto calibration:

    [TFTDMA] ret=0x101 (ESP_ERR_NO_MEM) len=16287 buf=0x3d800908 dmaCap=0

The LVGL draw buffers are full-screen TFT_eSprites, which createSprite() puts
in PSRAM; ESP32-S3 SPI master cannot DMA from PSRAM, so the driver tried to
allocate an internal bounce buffer per transfer and ran out of internal heap.

The real fix is caller-side (my_disp_flush/setupDMA stage each band through
internal DMA-capable RAM), so this path should no longer be reachable. The
patch is the safety net: for an instrument, dropping a frame always beats a
reset. It also logs the first few failures so a regression is visible.

Only the assert inside the 6-argument pushImageDMA is touched - the file has
several `assert(ret == ESP_OK)` lines and the others are left alone.

Idempotent: detects its own marker and does nothing if already applied.
"""

import os
import sys

TARGET = os.path.join("lib", "TFT_eSPI", "Processors", "TFT_eSPI_ESP32_S3.c")

# Start of the 6-arg overload whose assert we replace.
FUNC_ANCHOR = (
    "void TFT_eSPI::pushImageDMA(int32_t x, int32_t y, int32_t w, int32_t h, "
    "uint16_t* image, uint16_t* buffer)"
)
ASSERT_LINE = "assert(ret == ESP_OK);"
MARKER = "[TFTDMA]"  # presence means we already patched this file

REPLACEMENT = """  // PATCHED by patch_tft_espi.py (pre-build) - do not edit here; the submodule
  // is not covered by the parent repo's history, so edits are lost on
  // `git submodule update`. Change the script instead.
  //
  // Upstream does assert(ret == ESP_OK) here, which abort()s and REBOOTS the
  // device on any DMA queue failure while discarding the error code. Observed
  // for real as ESP_ERR_NO_MEM when DMAing from a PSRAM-backed LVGL buffer.
  // Dropping a frame is always preferable to resetting an instrument.
  if (ret != ESP_OK) {
    static uint32_t dmaErrCount = 0;
    if (++dmaErrCount <= 5) {
      Serial.printf("\\n[TFTDMA] ret=0x%x (%s) len=%u buf=%p dmaCap=%d - frame dropped\\n",
                    (int)ret, esp_err_to_name(ret), (unsigned)len, (void *)buffer,
                    (int)esp_ptr_dma_capable(buffer));
    }
    return; // nothing queued -> must NOT increment spiBusyCheck
  }"""


def main():
    if not os.path.isfile(TARGET):
        print("patch_tft_espi: WARNING %s not found - submodule not checked out?" % TARGET)
        print("patch_tft_espi: WARNING display DMA failures will abort() and reboot the board")
        return

    with open(TARGET, "r", encoding="utf-8", errors="surrogateescape") as fh:
        text = fh.read()

    if MARKER in text:
        print("patch_tft_espi: already patched, nothing to do")
        return

    lines = text.split("\n")

    func_idx = next((i for i, l in enumerate(lines) if FUNC_ANCHOR in l), None)
    if func_idx is None:
        print("patch_tft_espi: WARNING could not find pushImageDMA(6-arg) - NOT patched")
        print("patch_tft_espi: WARNING a failed display DMA will abort() and reboot the board")
        return

    # First bare assert(ret == ESP_OK); at or after the function start.
    assert_idx = next(
        (i for i in range(func_idx, len(lines)) if lines[i].strip() == ASSERT_LINE),
        None,
    )
    if assert_idx is None:
        print("patch_tft_espi: WARNING assert not found after pushImageDMA - NOT patched")
        print("patch_tft_espi: WARNING a failed display DMA will abort() and reboot the board")
        return

    lines[assert_idx] = REPLACEMENT

    with open(TARGET, "w", encoding="utf-8", errors="surrogateescape") as fh:
        fh.write("\n".join(lines))

    print("patch_tft_espi: patched %s:%d (fatal assert -> log + frame drop)"
          % (TARGET, assert_idx + 1))


main()
