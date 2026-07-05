# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Module Overview

OFM-Feedback is an **OpenKNX module** providing buzzer and vibration motor control. It is consumed as a sibling-repo symlink (`lib/OFM-Feedback`) in OpenKNX application projects. The module is registered in the host application's `main.cpp` as `openknx.addModule(N, openknxFeedback)`.

## Architecture

### Public API (`Feedback.h`)

All public methods default `iExternal = false` (firmware-triggered). KNX-triggered calls pass `iExternal = true` — this affects lock behavior.

**Single-tone overloads:**
```cpp
void setBuzzer(bool iOn);
void setBuzzer(bool iOn, uint32_t iDuration);      // ms, 0 = infinite
void setBuzzer(uint8_t iVolume, uint32_t iDuration); // BuzzerOff/Silent/Normal/Loud
void setBuzzer(uint16_t iFrequency, uint32_t iDuration); // Hz, 0 = off
```

**Tone-sequence overloads:**
```cpp
void setBuzzer(const BuzzerTone* iTones);              // pointer, Ende-Marker {0,0,0} required
void setBuzzer(std::initializer_list<BuzzerTone> iTones); // inline, no Ende-Marker needed
```

Every `setBuzzer` call immediately stops any running tone or sequence before starting the new one.

### BuzzerTone struct

```cpp
struct BuzzerTone {
    uint16_t frequency;     // Hz; 0 = Ende-Marker (pointer variant only)
    uint32_t playDuration;  // ms; 0 = infinite (no advance to next tone)
    uint32_t pauseDuration; // ms pause after this tone; 0 = next tone starts immediately
};
```

**Inline example (no Ende-Marker needed):**
```cpp
openknxFeedback.setBuzzer({{1000, 200, 100}, {1500, 300, 0}});
```

**Static array example (Ende-Marker required):**
```cpp
static const Feedback::BuzzerTone seq[] = {{1000, 200, 100}, {1500, 300, 0}, {0, 0, 0}};
openknxFeedback.setBuzzer(seq);
```

Maximum sequence length is 10 tones (`BUZZER_SEQUENCE_MAX`).

### Internal call hierarchy

```
Public setBuzzer(bool/uint8/uint16)
  └─ setBuzzer(uint16, bool, uint32)   ← resets sequence state
       └─ _buzzerHwApply()             ← hardware I/O

Public setBuzzer(BuzzerTone* / initializer_list)
  └─ setBuzzer(begin, end, bool)       ← copies into _buzzerSequence[]
       └─ _buzzerSeqStep()
            └─ _buzzerHwApply()        ← hardware I/O

loop() — timer expired
  └─ _buzzerHwApply(0, ...)            ← hardware I/O (no sequence reset)

loop() — sequence advance
  └─ _buzzerSeqStep()
       └─ _buzzerHwApply()             ← hardware I/O (no sequence reset)
```

`_buzzerHwApply` is the only place that touches the hardware pin and `buzzerTimer`. It must never be called from outside `_buzzerSeqStep` or `setBuzzer(uint16_t, ...)`.

### Lock logic

Both buzzer and vibration check a KNX lock object. The corrected logic:
```cpp
lLock = lLock && (iExternal || ParamBUZZ_BuzzerLockAlsoInternal);
```
- Lock always blocks external (KNX) calls.
- Lock also blocks internal (firmware) calls only when `LockAlsoInternal` is set.

### Hardware pins (defined in host project's `hardware.h`)

- `OPENKNX_BUZZER_PIN` — uses `tone()`/`noTone()` when `ParamBUZZ_BuzzerWithFrequency`, else `digitalWrite`
- `OPENKNX_VIBRATION_PIN` — binary `digitalWrite` only

### Console commands

Both `showHelp()` and `processCommand()` guard on `knx.configured()` and return early if not configured.

`showHelp()` registers only the top-level command `"buzz"` with a short description. `processCommand()` handles:
- `buzz` — prints help lines for all sub-commands
- `buzz play` — plays a predefined test sequence: `{{1800,300,300},{3200,500,500},{2400,100,0}}`

To add further sub-commands, add an `if (iCmd == "buzz ...")` branch in `processCommand()` and a `printHelpLine` for it in the `iCmd == "buzz"` block.
