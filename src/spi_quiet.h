#pragma once
#include <Arduino.h>
#include <atomic>

// Low-noise mode (adcRate 0, "~10 SPS"): display SPI traffic and ADC
// conversions never overlap. They take turns:
//   - Core 0 defers each conversion start while the display is drawing, and
//     after a conversion holds the next start briefly when Core 1 is waiting
//     to draw, so the display gets one slot per conversion.
//   - Core 1 skips a render (lv_timer_handler / lv_refr_now, which also does
//     the SPI touch read) while a conversion is in flight.
// In other modes lowNoise is false and spiQuietBegin() always returns true.
// Failsafe: if Core 1 has been refused for 250 ms (ADC stalled, no DRDY), it
// draws anyway so the UI can never freeze in this mode.
//
// adcConverting/spiActive form a Dekker-style claim: each side sets its own
// flag, then checks the other's (seq_cst), so both can never proceed at once.
namespace SpiQuiet
{
    inline std::atomic<bool> lowNoise{false};      // Core 0: adcRate == 0
    inline std::atomic<bool> adcConverting{false}; // Core 0 set, DRDY ISR clears
    inline std::atomic<bool> spiActive{false};     // Core 1: drawing / touch read
    inline std::atomic<bool> renderWanted{false};  // Core 1 was blocked by a conversion
    inline std::atomic<bool> adcTurn{false};       // Core 1 drew; next slot is the ADC's
    inline unsigned long lastGrant = 0;            // Core 1 only
}

// Core 1: call before any display/touch SPI. false = skip this pass.
inline bool spiQuietBegin()
{
    using namespace SpiQuiet;
    if (!lowNoise.load())
        return true;
    const bool stalled = millis() - lastGrant > 250;
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

// Core 1: call after the SPI work began by a successful spiQuietBegin().
inline void spiQuietEnd()
{
    using namespace SpiQuiet;
    spiActive.store(false);
    if (lowNoise.load())
        adcTurn.store(true);
    renderWanted.store(false);
}
