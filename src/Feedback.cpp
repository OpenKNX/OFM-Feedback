#include "Feedback.h"

void Feedback::setup()
{
// setup buzzer
#ifdef BUZZER_PIN
    logInfoP("Setting: Buzzer available: %d", ParamLOG_BuzzerInstalled);
    pinMode(BUZZER_PIN, OUTPUT);
#endif

}

void Feedback::loop()
{

}

// on input level, all dpt > 1 values are converted to bool by the according converter
void Feedback::processInputKo(GroupObject &iKo)
{
#ifdef BUZZER_PIN
    if (iKo.asap() == BUZZ_KoBuzzerLock)
    {
        // turn off buzzer in case of lock
        if (ParamBUZZ_BuzzerLock > 0 && (bool)iKo.value(DPT_Switch) == (ParamBUZZ_BuzzerLock == 1))
            noTone(BUZZER_PIN);
    }
#endif
}

// turn on/off Buzzer
void Feedback::setBuzzer()
{
#ifdef BUZZER_PIN
    // check for global lock and alarm
    if (ParamBUZZ_BuzzerLock == 0 || (bool)KoBUZZ_BuzzerLock.value(DPT_Switch) != (ParamBUZZ_BuzzerLock == 1))
    {
        switch (ParamBUZZ_BuzzerVolumeExternal)
        {
            case BuzzerOff:
                noTone(BUZZER_PIN);
                break;
            case BuzzerLoud:
                tone(BUZZER_PIN, ParamBUZZ_BuzzerLoud);
                break;
            case BuzzerSilent:
                tone(BUZZER_PIN, ParamBUZZ_BuzzerSilent);
                break;
            case BuzzerNormal:
                tone(BUZZER_PIN, ParamBUZZ_BuzzerNormal);
                break;
            default:
                break;
        }
    }
    else
    {
        // in case of lock we turn off buzzer
        noTone(BUZZER_PIN);
    }
#endif
}


/* Feedback Instance */
Feedback openknxfeedback;