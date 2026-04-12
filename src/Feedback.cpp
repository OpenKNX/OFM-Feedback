#include "Feedback.h"

void Feedback::setup()
{
// setup buzzer
#ifdef OPENKNX_BUZZER_PIN
    logInfoP("Buzzer available: %d", ParamBUZZ_BuzzerInstalled);
    if (ParamBUZZ_BuzzerInstalled) {
        pinMode(OPENKNX_BUZZER_PIN, OUTPUT);
        KoBUZZ_BuzzerState.valueNoSend(false, DPT_Switch);
    }
#endif
// setup vibration
#ifdef OPENKNX_VIBRATION_PIN
    logInfoP("Vibration available: %d", ParamBUZZ_VibrationInstalled);
    if (ParamBUZZ_VibrationInstalled) {
        pinMode(OPENKNX_VIBRATION_PIN, OUTPUT);
        KoBUZZ_VibrationState.valueNoSend(false, DPT_Switch);
    }
#endif
}

void Feedback::loop()
{
    if (buzzerTimer > 0) 
    {
        uint32_t lBuzzerTimeout = buzzerModeExternal ? ParamBUZZ_BuzzerExtTimeMS : ParamBUZZ_BuzzerIntTimeMS;
        if (delayCheck(buzzerTimer, lBuzzerTimeout)) 
            setBuzzer(false, buzzerModeExternal);
    }

    if (vibrationTimer > 0) 
    {
        uint32_t lVibrationTimeout = vibrationModeExternal ? ParamBUZZ_VibrationExtTimeMS : ParamBUZZ_VibrationIntTimeMS;
        if (delayCheck(vibrationTimer, lVibrationTimeout)) 
            setVibration(false, vibrationModeExternal);
    }
}

// on input level, all dpt > 1 values are converted to bool by the according converter
void Feedback::processInputKo(GroupObject &iKo)
{
    switch (iKo.asap())
    {
        case BUZZ_KoBuzzerLock:
            // turn off buzzer in case of lock
            if (ParamBUZZ_BuzzerLock > 0 && (bool)iKo.value(DPT_Switch) == (ParamBUZZ_BuzzerLock == 1))
                setBuzzer(false, true);
            break;
        case BUZZ_KoVibrationLock:
            // turn off vibration in case of lock
            if (ParamBUZZ_VibrationLock > 0 && (bool)iKo.value(DPT_Switch) == (ParamBUZZ_VibrationLock == 1))
                setVibration(false, true);
            break;
        case BUZZ_KoBuzzerActive:
            switch (ParamBUZZ_BuzzerSetup)
            {
                case BKO_Switch:
                    setBuzzer((bool)iKo.value(DPT_Switch), true);
                    break;
                case BKO_Volume:
                    setBuzzer((uint8_t)iKo.value(DPT_DecimalFactor), true);
                    break;
                case BKO_Frequency:
                    setBuzzer((uint16_t)iKo.value(DPT_Value_Frequency), true);
                    break;
                default:
                    // do nothing
                    break;
            }
        case BUZZ_KoVibrationActive:
            setVibration((bool)iKo.value(DPT_Switch), true);
            break;
    }
}

void Feedback::setBuzzer(bool iOn, bool iExternal)
{ 
    if (iOn)
        if (ParamBUZZ_BuzzerWithFrequency)
            setBuzzer((uint8_t)(iExternal ? ParamBUZZ_BuzzerVolumeExternal : ParamBUZZ_BuzzerVolumeInternal), iExternal);
        else
            setBuzzer((uint8_t)BuzzerNormal, iExternal);
    else
        setBuzzer((uint16_t)0, iExternal);
}

void Feedback::setBuzzer(uint8_t iVolume, bool iExternal)
{
    switch (iVolume)
    {
        case BuzzerOff:
            setBuzzer((uint16_t)0, iExternal);
            break;
        case BuzzerSilent:
            setBuzzer(ParamBUZZ_BuzzerSilent, iExternal);
            break;
        case BuzzerNormal:
            if (ParamBUZZ_BuzzerWithFrequency)
                setBuzzer(ParamBUZZ_BuzzerNormal, iExternal);
            else
                setBuzzer((uint16_t)1500, iExternal);
            break;
        case BuzzerLoud:
            setBuzzer(ParamBUZZ_BuzzerLoud, iExternal);
            break;
        default:
            // do nothing
            break;
    }
}

// turn on/off Buzzer
void Feedback::setBuzzer(uint16_t iFrequency, bool iExternal)
{
#ifdef OPENKNX_BUZZER_PIN
    if (ParamBUZZ_BuzzerInstalled) 
    {
        // check for global lock 
        bool lLock = ParamBUZZ_BuzzerLock > 0 && (bool)KoBUZZ_BuzzerLock.value(DPT_Switch) == (ParamBUZZ_BuzzerLock == 1);
        // check lock external/internal 
        lLock = lLock && (ParamBUZZ_BuzzerLockAlsoInternal != iExternal);
        if (iFrequency == 0)
        {
            if (ParamBUZZ_BuzzerWithFrequency)
                noTone(OPENKNX_BUZZER_PIN);
            else
                digitalWrite(OPENKNX_BUZZER_PIN, LOW);
            KoBUZZ_BuzzerState.value(false, DPT_Switch);
            buzzerTimer = 0;
            logDebugP("Buzzer OFF");
        } 
        else if (!lLock &&iFrequency >= 500 && iFrequency <= 6000) 
        {
            if (ParamBUZZ_BuzzerWithFrequency)
            {
                tone(OPENKNX_BUZZER_PIN, iFrequency);
                logDebugP("Buzzer ON: %d Hz (External: %d)", iFrequency, iExternal);
            }
            else
            {
                digitalWrite(OPENKNX_BUZZER_PIN, HIGH);
                logDebugP("Buzzer ON (External: %d)", iExternal);
            }
            KoBUZZ_BuzzerState.value(true, DPT_Switch);
            buzzerTimer = delayTimerInit();
            buzzerModeExternal = iExternal;
        }
    }
#endif
}

void Feedback::setVibration(bool iOn, bool iExternal)
{
#ifdef OPENKNX_VIBRATION_PIN
    if (ParamBUZZ_VibrationInstalled) 
    {
        // check for global lock 
        bool lLock = ParamBUZZ_VibrationLock > 0 && (bool)KoBUZZ_VibrationLock.value(DPT_Switch) == (ParamBUZZ_VibrationLock == 1);
        // check lock external/internal 
        lLock = lLock && (ParamBUZZ_VibrationLockAlsoInternal != iExternal);
        if (iOn && !lLock)
        {
            digitalWrite(OPENKNX_VIBRATION_PIN, HIGH);
            KoBUZZ_VibrationState.value(true, DPT_Switch);
            vibrationTimer = delayTimerInit();
            vibrationModeExternal = iExternal;
            logDebugP("Vibration ON (External: %d)", iExternal);
        } else {
            digitalWrite(OPENKNX_VIBRATION_PIN, LOW);
            KoBUZZ_VibrationState.value(false, DPT_Switch);
            vibrationTimer = 0;
            logDebugP("Vibration OFF (External: %d)", iExternal);
        }
    }
#endif
}

/* Feedback Instance */
Feedback openknxFeedback;