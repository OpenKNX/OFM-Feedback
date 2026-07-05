#pragma once
#include <initializer_list>
#include "OpenKNX.h"

#ifndef OPENKNX_BUZZER_PIN
  #ifdef BUZZER_PIN
      #define OPENKNX_BUZZER_PIN BUZZER_PIN 
  #endif
#endif

class Feedback : public OpenKNX::Module
{
  public:
    struct BuzzerTone {
        uint16_t frequency;     // Hz; 0 = Ende-Marker
        uint32_t playDuration;  // ms; 0 = unendlich (kein Weiterschalten)
        uint32_t pauseDuration; // ms; 0 = sofort nächster Ton
    };

  private:
  enum BuzzerVolume
  {
      BuzzerOff,    
      BuzzerSilent, 
      BuzzerNormal, 
      BuzzerLoud   
  };

  enum BuzzerKoType
  {
      BKO_Switch,
      BKO_Volume,
      BKO_Frequency
  };

  public:
    Feedback() {};

    const std::string version() override { return MODULE_Feedback_Version; }
    virtual const std::string name() override { return "Feedback"; };
    void processInputKo(GroupObject &iKo) override;
    virtual void setup() override;
    virtual void loop() override;
    bool processCommand(const std::string iCmd, bool iDebugKo) override;
    void showHelp() override;
    void setBuzzer(bool iOn) { setBuzzer(iOn, false); };
    void setBuzzer(bool iOn, uint32_t iDuration = 0) { setBuzzer(iOn, false, iDuration); };
    void setBuzzer(uint8_t iVolume, uint32_t iDuration = 0) { setBuzzer(iVolume, false, iDuration); };
    void setBuzzer(uint16_t iFrequency, uint32_t iDuration = 0) { setBuzzer(iFrequency, false, iDuration); };
    void setBuzzer(const BuzzerTone* iTones) { setBuzzer(iTones, false); };
    void setBuzzer(std::initializer_list<BuzzerTone> iTones) { setBuzzer(iTones.begin(), iTones.end(), false); };

    void setVibration(bool iOn) { setVibration(iOn, false); };

  private:    
  
    void setBuzzer(bool iOn, bool iExternal, uint32_t iDuration = 0);
    void setBuzzer(uint8_t iVolume, bool iExternal, uint32_t iDuration = 0);
    void setBuzzer(uint16_t iFrequency, bool iExternal, uint32_t iDuration = 0);
    void setBuzzer(const BuzzerTone* iTones, bool iExternal);
    void setBuzzer(const BuzzerTone* iBegin, const BuzzerTone* iEnd, bool iExternal);
    void _buzzerSeqStep();
    void _buzzerHwApply(uint16_t iFrequency, bool iExternal, uint32_t iDuration);

    void setVibration(bool iOn, bool iExternal, uint32_t iDuration = 0);

    // runtime state for buzzer
    bool buzzerModeExternal = false; // false=internal, true=external
    uint32_t buzzerTimer = 0;
    uint32_t buzzerDuration = 0;

    // runtime state for buzzer sequence
    static constexpr uint8_t BUZZER_SEQUENCE_MAX = 10;
    BuzzerTone _buzzerSequence[BUZZER_SEQUENCE_MAX + 1];
    uint8_t _buzzerSeqIndex = 0;
    bool _buzzerInPause = false;
    uint32_t _buzzerSeqTimer = 0;

    // runtime state for vibration
    bool vibrationModeExternal = false; // false=internal, true=external
    uint32_t vibrationTimer = 0;
    uint32_t vibrationDuration = 0;
};

extern Feedback openknxFeedback;