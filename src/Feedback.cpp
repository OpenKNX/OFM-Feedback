#include "Feedback.h"

void Feedback::showHelp()
{
    if (!knx.configured())
        return;

    openknx.console.printHelpLine("buzz", "Buzzer commands");
}

bool Feedback::processCommand(const std::string iCmd, bool iDebugKo)
{
    if (!knx.configured())
        return false;

    if (iCmd.substr(0, 4) != "buzz")
        return false;

    if (iCmd == "buzz")
    {
        openknx.console.printHelpLine("buzz play", "Play a test tone sequence");
        return true;
    }

    if (iCmd == "buzz play")
    {
        setBuzzer({{1800, 300, 300}, {3200, 500, 500}, {2400, 100, 0}});
        return true;
    }

    return false;
}

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
        if (buzzerDuration > 0)
            lBuzzerTimeout = buzzerDuration;
        if (delayCheck(buzzerTimer, lBuzzerTimeout))
            _buzzerHwApply(0, buzzerModeExternal, 0);
    }

    if (_buzzerSequence[_buzzerSeqIndex].frequency != 0 || _buzzerInPause)
    {
        if (!_buzzerInPause && buzzerTimer == 0)
        {
            uint32_t pause = _buzzerSequence[_buzzerSeqIndex].pauseDuration;
            _buzzerSeqIndex++;
            if (pause > 0)
            {
                _buzzerInPause = true;
                _buzzerSeqTimer = delayTimerInit();
            }
            else
            {
                _buzzerSeqStep();
            }
        }
        else if (_buzzerInPause && delayCheck(_buzzerSeqTimer, _buzzerSequence[_buzzerSeqIndex - 1].pauseDuration))
        {
            _buzzerInPause = false;
            _buzzerSeqStep();
        }
    }

    if (vibrationTimer > 0) 
    {
        uint32_t lVibrationTimeout = vibrationModeExternal ? ParamBUZZ_VibrationExtTimeMS : ParamBUZZ_VibrationIntTimeMS;
        if (vibrationDuration > 0)
            lVibrationTimeout = vibrationDuration;
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
            break;
        case BUZZ_KoVibrationActive:
            setVibration((bool)iKo.value(DPT_Switch), true);
            break;
    }
}

void Feedback::setBuzzer(bool iOn, bool iExternal, uint32_t iDuration)
{ 
    if (iOn)
        if (ParamBUZZ_BuzzerWithFrequency)
            setBuzzer((uint8_t)(iExternal ? ParamBUZZ_BuzzerVolumeExternal : ParamBUZZ_BuzzerVolumeInternal), iExternal, iDuration);
        else
            setBuzzer((uint8_t)BuzzerNormal, iExternal, iDuration);
    else
        setBuzzer((uint16_t)0, iExternal, iDuration);
}

void Feedback::setBuzzer(uint8_t iVolume, bool iExternal, uint32_t iDuration)
{
    switch (iVolume)
    {
        case BuzzerOff:
            setBuzzer((uint16_t)0, iExternal, iDuration);
            break;
        case BuzzerSilent:
            setBuzzer(ParamBUZZ_BuzzerSilent, iExternal, iDuration);
            break;
        case BuzzerNormal:
            if (ParamBUZZ_BuzzerWithFrequency)
                setBuzzer(ParamBUZZ_BuzzerNormal, iExternal, iDuration);
            else
                setBuzzer((uint16_t)1500, iExternal, iDuration);
            break;
        case BuzzerLoud:
            setBuzzer(ParamBUZZ_BuzzerLoud, iExternal, iDuration);
            break;
        default:
            // do nothing
            break;
    }
}

void Feedback::setBuzzer(const BuzzerTone* iTones, bool iExternal)
{
    uint8_t i = 0;
    while (i < BUZZER_SEQUENCE_MAX && iTones[i].frequency != 0)
        i++;
    setBuzzer(iTones, iTones + i, iExternal);
}

void Feedback::setBuzzer(const BuzzerTone* iBegin, const BuzzerTone* iEnd, bool iExternal)
{
    _buzzerSeqIndex = 0;
    _buzzerInPause = false;
    _buzzerSeqTimer = 0;

    uint8_t i = 0;
    while (i < BUZZER_SEQUENCE_MAX && iBegin + i < iEnd)
    {
        _buzzerSequence[i] = iBegin[i];
        i++;
    }
    _buzzerSequence[i] = {0, 0, 0};

    buzzerModeExternal = iExternal;
    _buzzerSeqStep();
}

void Feedback::_buzzerSeqStep()
{
    const BuzzerTone& tone = _buzzerSequence[_buzzerSeqIndex];
    if (tone.frequency == 0)
    {
        _buzzerHwApply(0, buzzerModeExternal, 0);
        return;
    }
    _buzzerHwApply(tone.frequency, buzzerModeExternal, tone.playDuration);
}

// turn on/off Buzzer — resets any running sequence
void Feedback::setBuzzer(uint16_t iFrequency, bool iExternal, uint32_t iDuration)
{
    _buzzerSeqIndex = 0;
    _buzzerInPause = false;
    _buzzerSeqTimer = 0;
    _buzzerSequence[0] = {0, 0, 0};
    _buzzerHwApply(iFrequency, iExternal, iDuration);
}

void Feedback::_buzzerHwApply(uint16_t iFrequency, bool iExternal, uint32_t iDuration)
{
#ifdef OPENKNX_BUZZER_PIN
    if (ParamBUZZ_BuzzerInstalled) 
    {
        // check for global lock 
        bool lLock = ParamBUZZ_BuzzerLock > 0 && (bool)KoBUZZ_BuzzerLock.value(DPT_Switch) == (ParamBUZZ_BuzzerLock == 1);
        // check lock external/internal 
        lLock = lLock && (iExternal || ParamBUZZ_BuzzerLockAlsoInternal);
        if (iFrequency == 0)
        {
            if (ParamBUZZ_BuzzerWithFrequency)
                noTone(OPENKNX_BUZZER_PIN);
            else
                digitalWrite(OPENKNX_BUZZER_PIN, LOW);
            KoBUZZ_BuzzerState.value(false, DPT_Switch);
            buzzerTimer = 0;
            buzzerDuration = 0;
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
            buzzerDuration = iDuration;
            buzzerModeExternal = iExternal;
        }
    }
#endif
}

void Feedback::setVibration(bool iOn, bool iExternal, uint32_t iDuration)
{
#ifdef OPENKNX_VIBRATION_PIN
    if (ParamBUZZ_VibrationInstalled) 
    {
        // check for global lock 
        bool lLock = ParamBUZZ_VibrationLock > 0 && (bool)KoBUZZ_VibrationLock.value(DPT_Switch) == (ParamBUZZ_VibrationLock == 1);
        // check lock external/internal 
        lLock = lLock && (iExternal || ParamBUZZ_VibrationLockAlsoInternal);
        if (iOn && !lLock)
        {
            digitalWrite(OPENKNX_VIBRATION_PIN, HIGH);
            KoBUZZ_VibrationState.value(true, DPT_Switch);
            vibrationTimer = delayTimerInit();
            vibrationModeExternal = iExternal;
            vibrationDuration = iDuration;
            logDebugP("Vibration ON (External: %d)", iExternal);
        } else {
            digitalWrite(OPENKNX_VIBRATION_PIN, LOW);
            KoBUZZ_VibrationState.value(false, DPT_Switch);
            vibrationTimer = 0;
            vibrationDuration = 0;
            logDebugP("Vibration OFF (External: %d)", iExternal);
        }
    }
#endif
}

/* Feedback Instance */
Feedback openknxFeedback;