#pragma once
#include "OpenKNX.h"

#ifndef OPENKNX_BUZZER_PIN
  #ifdef BUZZER_PIN
      #define OPENKNX_BUZZER_PIN BUZZER_PIN 
  #endif
#endif

class Feedback : public OpenKNX::Module
{
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
    void setBuzzer(bool iOn) { setBuzzer(iOn, false); };
    void setVibration(bool iOn) { setVibration(iOn, false); };
    void setBuzzer(bool iOn, bool iExternal, uint32_t iDuration = 0);
    void setBuzzer(uint8_t iVolume, bool iExternal, uint32_t iDuration = 0);
    void setBuzzer(uint16_t iFrequency, bool iExternal, uint32_t iDuration = 0);
    void setVibration(bool iOn, bool iExternal, uint32_t iDuration = 0);

  private:    
    // runtime state for buzzer
    bool buzzerModeExternal = false; // false=internal, true=external
    uint32_t buzzerTimer = 0;
    uint32_t buzzerDuration = 0;

    // runtime state for vibration
    bool vibrationModeExternal = false; // false=internal, true=external
    uint32_t vibrationTimer = 0;
    uint32_t vibrationDuration = 0;
};

extern Feedback openknxFeedback;