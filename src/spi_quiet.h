#pragma once
#include <Arduino.h>
#include <atomic>

// Low-noise mode (adcRate 0, "~10 SPS"): display/touch SPI and ADC conversions
// never overlap.
//   - Core 0 holds adcConverting for a whole V+I pair (ADC::tryStartPending /
//     endPair). At the pair boundary it waits briefly if Core 1 wants a slot.
//   - Core 1 renders into the draw buffer (CPU only) while the ADC converts;
//     only the SPI push waits: my_disp_flush() claims the slot on the frame's
//     first flush (spiFrameClaim, blocking) and releases it after the last one,
//     sampling touch inside the same slot.
// In other modes lowNoise is false and everything here passes straight through.
// Failsafe: if Core 1 has been refused for 600 ms (ADC stalled, no DRDY), it
// goes ahead anyway so the UI can never freeze in this mode.
//
// adcConverting/spiActive form a Dekker-style claim: each side sets its own
// flag, then checks the other's (seq_cst), so both can never proceed at once.
namespace SpiQuiet
{
    inline std::atomic<bool> lowNoise{false};      // Core 0: adcRate == 0
    inline std::atomic<bool> adcConverting{false}; // Core 0: V+I pair in progress
    inline std::atomic<bool> spiActive{false};     // Core 1: drawing / touch read
    inline std::atomic<bool> renderWanted{false};  // Core 1 is waiting for a slot
    inline std::atomic<bool> adcTurn{false};       // Core 1 drew; next slot is the ADC's
    inline unsigned long lastGrant = 0;            // Core 1 only
    inline bool frameClaimed = false;              // Core 1 only
}

// Core 1: try to take the SPI slot without waiting. false = not now.
inline bool spiQuietBegin()
{
    using namespace SpiQuiet;
    if (!lowNoise.load())
        return true;
    const bool stalled = millis() - lastGrant > 600;
    if (adcTurn.load() && !stalled)
        return false;
    spiActive.store(true);
    if (adcConverting.load() && !stalled)
    {
        spiActive.store(false);
        renderWanted.store(true);
        return false;
    }
    lastGrant = millis();
    return true;
}

// Core 1: release a slot taken by spiQuietBegin()/spiQuietAcquire().
inline void spiQuietEnd()
{
    using namespace SpiQuiet;
    spiActive.store(false);
    if (lowNoise.load())
        adcTurn.store(true);
    renderWanted.store(false);
}

// Core 1: wait for the SPI slot (at most one V+I pair, or the failsafe).
inline void spiQuietAcquire()
{
    while (!spiQuietBegin())
        vTaskDelay(1);
}

// my_disp_flush(): claim on a frame's first flush, release after its last.
inline void spiFrameClaim()
{
    if (!SpiQuiet::frameClaimed)
    {
        spiQuietAcquire();
        SpiQuiet::frameClaimed = true;
    }
}

// Also called after each lv_timer_handler()/lv_refr_now() as a safety net, so
// a frame that never reported its last flush can't hold the ADC off.
inline void spiFrameRelease()
{
    if (SpiQuiet::frameClaimed)
    {
        SpiQuiet::frameClaimed = false;
        spiQuietEnd();
    }
}
