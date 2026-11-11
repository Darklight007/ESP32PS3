// #include "pitches.h"
#include "buzzer.h"
#include "driver/timer.h"
#include "soc/gpio_struct.h"

unsigned long toneTime[2];
bool toneIsOn = true;
bool buzzerSound = true;
static int lastFreq = 0; // frequency the LEDC timer was last set up for (myTone)

// Initialize buzzer LEDC peripheral - must be called during setup
void initBuzzer()
{
    ledcSetup(SOUND_PWM_CHANNEL, 1000, SOUND_RESOLUTION); // Default 1kHz frequency
    ledcAttachPin(BUZZER_PIN, SOUND_PWM_CHANNEL);
    ledcWrite(SOUND_PWM_CHANNEL, SOUND_OFF); // Start silent
}

void myTone(int &&frequency, int &&duration)
{
    if (!buzzerSound)
    {
        ledcWrite(SOUND_PWM_CHANNEL, SOUND_OFF);
        return;
    }

    toneTime[1] = duration;
    toneIsOn = true;

    // Only reconfigure if frequency changed (avoid slow ledcSetup on every call)
    if (frequency != lastFreq) {
        ledcSetup(SOUND_PWM_CHANNEL, frequency, SOUND_RESOLUTION);
        lastFreq = frequency;
    }
    ledcWrite(SOUND_PWM_CHANNEL, SOUND_ON);
    toneTime[0] = millis();
}

void myTone(int frequency, int duration, bool blockingDelay)
{
    if (!buzzerSound)
    {
        ledcWrite(SOUND_PWM_CHANNEL, SOUND_OFF);
        return;
    }

    toneTime[0] = millis() + duration;
    ledcSetup(SOUND_PWM_CHANNEL, frequency, SOUND_RESOLUTION); // Set up PWM channel
    // Pin already attached in initBuzzer(), no need to reattach
    ledcWrite(SOUND_PWM_CHANNEL, SOUND_ON);
    delay(duration);
    ledcWrite(SOUND_PWM_CHANNEL, SOUND_OFF);
    toneIsOn = true;
}

// Sweeps (siren): a hardware timer toggles the buzzer pin itself, and a pitch
// change only alters the next half-period, so the wave never breaks. Changing
// the LEDC divider or duty at every step clicked ("water falling" on the
// fall), and rebuilding the LEDC timer 50x/s divide-by-zero'd inside IDF.
// The pin is taken from LEDC for the sweep and handed back by sweepStop().
static constexpr timer_group_t kSweepGroup = TIMER_GROUP_1;
static constexpr timer_idx_t kSweepTimer = TIMER_1;
static constexpr uint32_t kSweepTickHz = 10000000; // APB 80 MHz / 8
static volatile uint32_t s_ticksHigh = 0, s_ticksLow = 0;
static volatile bool s_pinHigh = false;
static bool s_sweepOn = false;

static bool IRAM_ATTR sweepIsr(void *)
{
    s_pinHigh = !s_pinHigh;
    if (s_pinHigh)
        GPIO.out1_w1ts.val = 1u << (BUZZER_PIN - 32);
    else
        GPIO.out1_w1tc.val = 1u << (BUZZER_PIN - 32);
    timer_group_set_alarm_value_in_isr(kSweepGroup, kSweepTimer, s_pinHigh ? s_ticksHigh : s_ticksLow);
    return false;
}

void sweepSet(float frequency, float volume)
{
    frequency = constrain(frequency, 100.0f, 10000.0f);
    volume = constrain(volume, 0.0f, 1.0f);
    const uint32_t period = (uint32_t)(kSweepTickHz / frequency);
    // Quieter = shorter high time (as the LEDC duty did); keep both halves
    // long enough that the ISR rate stays at 2 x pitch.
    uint32_t high = (uint32_t)(period * 0.5f * volume);
    high = constrain(high, 20u, period - 20);
    s_ticksHigh = high;
    s_ticksLow = period - high;
}

void sweepStart(float frequency)
{
    if (!buzzerSound || s_sweepOn)
        return;
    sweepSet(frequency, 1);
    ledcDetachPin(BUZZER_PIN);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    s_pinHigh = false;
    timer_config_t cfg = {};
    cfg.alarm_en = TIMER_ALARM_EN;
    cfg.counter_en = TIMER_PAUSE;
    cfg.intr_type = TIMER_INTR_LEVEL;
    cfg.counter_dir = TIMER_COUNT_UP;
    cfg.auto_reload = TIMER_AUTORELOAD_EN;
    cfg.divider = 80000000 / kSweepTickHz;
    timer_init(kSweepGroup, kSweepTimer, &cfg);
    timer_set_counter_value(kSweepGroup, kSweepTimer, 0);
    timer_set_alarm_value(kSweepGroup, kSweepTimer, s_ticksLow);
    timer_isr_callback_add(kSweepGroup, kSweepTimer, sweepIsr, nullptr, 0);
    timer_start(kSweepGroup, kSweepTimer);
    s_sweepOn = true;
}

void sweepStop()
{
    if (!s_sweepOn)
        return;
    timer_pause(kSweepGroup, kSweepTimer);
    timer_isr_callback_remove(kSweepGroup, kSweepTimer);
    timer_deinit(kSweepGroup, kSweepTimer);
    digitalWrite(BUZZER_PIN, LOW);
    ledcAttachPin(BUZZER_PIN, SOUND_PWM_CHANNEL);
    s_sweepOn = false;
}

void toneOff()
{
    // if (!buzzerSound)
    // return;

    // toneIsOn guard: this runs on every Task_ADC pass (~1000x/s); without it
    // the PWM was rewritten (two LEDC driver calls) even when silent.
    if (toneIsOn && (toneTime[1]+ toneTime[0]) <millis())
    {
        // if ((millis() - toneTime[0]) > toneTime[1])
        ledcWrite(SOUND_PWM_CHANNEL, SOUND_OFF);
        toneIsOn = false;

        // Serial.printf("\nBeep at %ihz for %i ms. ",frequency,duration);
    }
}
