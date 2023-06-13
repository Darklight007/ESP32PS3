// #include "pitches.h"
#include "buzzer.h"

unsigned long toneTime[2];
bool toneIsOn = true;
bool buzzerSound = true;

void myTone(int &&frequency, int &&duration)
{
    if (!buzzerSound)
    {
        ledcWrite(SOUND_PWM_CHANNEL, SOUND_OFF);
        return;
    }
    // if (toneIsOn && toneTime[1]>10)
    //     // toneTime[1] = duration;
    //     ;
    // else
        toneTime[1] = duration;


    toneIsOn = true;
    ledcSetup(SOUND_PWM_CHANNEL, frequency, SOUND_RESOLUTION); // Set up PWM channel
    // ledcAttachPin(BUZZER_PIN, SOUND_PWM_CHANNEL);              // Attach channel to pin
    ledcWrite(SOUND_PWM_CHANNEL, SOUND_ON);
    //  delay(duration);
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
    ledcAttachPin(BUZZER_PIN, SOUND_PWM_CHANNEL);              // Attach channel to pin
    ledcWrite(SOUND_PWM_CHANNEL, SOUND_ON);
    delay(duration);
    ledcWrite(SOUND_PWM_CHANNEL, SOUND_OFF);
    toneIsOn = true;
}

void toneOff()
{
    // if (!buzzerSound)
    // return;

    if (/*toneIsOn &&*/ (toneTime[1]+ toneTime[0]) <millis())
    {
        // if ((millis() - toneTime[0]) > toneTime[1])
        ledcWrite(SOUND_PWM_CHANNEL, SOUND_OFF);
        toneIsOn = false;

        // Serial.printf("\nBeep at %ihz for %i ms. ",frequency,duration);
    }
}
