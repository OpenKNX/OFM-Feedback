# OFM-Feedback

Implements acoustic (buzzer) and haptic (vibration motor) feedback for OpenKNX devices.

## Hardware setup

Define the following pins in your project's `hardware.h`:

```cpp
#define OPENKNX_BUZZER_PIN      <pin>   // buzzer, used with Arduino tone()/digitalWrite()
#define OPENKNX_VIBRATION_PIN   <pin>   // vibration motor, binary GPIO
```

## ETS defaults

The following ETS parameter defaults can be overridden in the host application's XML:

```xml
<op:config name="%BUZZ_BuzzerFreqQuiet%"       value="1800" />  <!-- frequency for a quiet tone -->
<op:config name="%BUZZ_BuzzerFreqNormal%"       value="3200" />  <!-- frequency for a normal tone -->
<op:config name="%BUZZ_BuzzerFreqLoud%"         value="2400" />  <!-- frequency for a loud tone (resonance frequency) -->
<op:config name="%BUZZ_BuzzerFreqMin%"          value="1500" />  <!-- minimum allowed frequency -->
<op:config name="%BUZZ_BuzzerFreqMax%"          value="6000" />  <!-- maximum allowed frequency -->
<op:config name="%BUZZ_BuzzerDurationInt%"      value="1"    />  <!-- buzz duration for internal calls (s) -->
<op:config name="%BUZZ_VibrationDurationInt%"   value="2"    />  <!-- vibration duration for internal calls (s) -->
```

## API

The global instance is `openknxFeedback`. All methods are firmware-internal calls (not KNX-triggered), so lock behavior follows the `BuzzerLockAlsoInternal` / `VibrationLockAlsoInternal` ETS parameters.

Every `setBuzzer` call immediately cancels any currently playing tone or sequence.

> **Note:** Buzzer and vibration output only occurs when the respective hardware is enabled in the ETS parameters (`BuzzerInstalled` / `VibrationInstalled`). Calls are silently ignored otherwise.

---

### setBuzzer — single tone

```cpp
void setBuzzer(bool iOn);
```
Turns the buzzer on or off. Uses the ETS-configured volume and duration for internal calls.

```cpp
void setBuzzer(bool iOn, uint32_t iDuration);
```
Same, but overrides the auto-off duration. `iDuration = 0` plays indefinitely until stopped by another `setBuzzer` call.

```cpp
void setBuzzer(uint8_t iVolume, uint32_t iDuration = 0);
```
Plays at a predefined volume level:

| Value | Meaning |
|---|---|
| `0` | Off |
| `1` | Silent (ETS parameter `BuzzerFreqQuiet`) |
| `2` | Normal (ETS parameter `BuzzerFreqNormal`) |
| `3` | Loud (ETS parameter `BuzzerFreqLoud`) |

```cpp
void setBuzzer(uint16_t iFrequency, uint32_t iDuration = 0);
```
Plays at an explicit frequency in Hz (valid range: 500–6000 Hz). `iFrequency = 0` turns the buzzer off.

---

### setBuzzer — tone sequence

Plays a sequence of tones with individual durations and pauses between them. The sequence advances automatically in `loop()`.

#### BuzzerTone struct

```cpp
struct Feedback::BuzzerTone {
    uint16_t frequency;     // Hz (500–6000); 0 = end marker (pointer variant only)
    uint32_t playDuration;  // ms; 0 = play indefinitely (sequence stops here)
    uint32_t pauseDuration; // ms pause after this tone; 0 = next tone starts immediately
};
```

#### Inline initializer list (recommended)

No end marker needed — the list knows its own length:

```cpp
openknxFeedback.setBuzzer({{1000, 200, 100}, {1500, 300, 0}});
```

#### Static array

Requires a `{0, 0, 0}` end marker. Maximum 10 tones:

```cpp
static const Feedback::BuzzerTone seq[] = {
    {1000, 200, 100},
    {1500, 300,   0},
    {   0,   0,   0}  // end marker
};
openknxFeedback.setBuzzer(seq);
```

---

### setVibration

```cpp
void setVibration(bool iOn);
```
Turns the vibration motor on or off. Duration is controlled by the ETS parameter `VibrationDurationInt`.

---

## Console commands

| Command | Description |
|---|---|
| `buzz` | List available sub-commands |
| `buzz play` | Play a test tone sequence (requires `BuzzerInstalled` in ETS) |
