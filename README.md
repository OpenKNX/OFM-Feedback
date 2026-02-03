# OFM-Feedback

Implements haptic or acoustic feedback  for OpenKNX devices

Following pin definitions are required:

* OPENKNX_BUZZER_PIN - defines the pin of the connected buzzer, used with arduino tone command
* OPENKNX_VIBRATION_PIN - defines the pin of the connected vibration motor, used with a binary GPIO

You can set the following defaults for the ETS application. The following values are preset, of no external defaults are set:

    <op:config name="%BUZZ_BuzzerFreqQuiet%" value="1800" />   - frequency for a quiet tone 
    <op:config name="%BUZZ_BuzzerFreqNormal%" value="3200" />  - frequency for a normal tone
    <op:config name="%BUZZ_BuzzerFreqLoud%" value="2400" />    - frequency for a loud tone (usually resonance frequence of the used buzzer)
    <op:config name="%BUZZ_BuzzerFreqMin%" value="1500" />     - minimum allowed frequency for the used buzzer
    <op:config name="%BUZZ_BuzzerFreqMax%" value="6000" />     - maximum allowed frequency for the used buzzer
    <op:config name="%BUZZ_BuzzerDurationInt%" value="1" />    - Buzz-duration for internal usage
    <op:config name="%BUZZ_VibrationDurationInt%" value="2" /> - Vibration-duration for internal usage

