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

  private:
    void setBuzzer(bool iOn, bool iExternal);
    void setBuzzer(uint8_t iVolume, bool iExternal);
    void setBuzzer(uint16_t iFrequency, bool iExternal);
    void setVibration(bool iOn, bool iExternal);
    
    // runtime state for buzzer
    bool buzzerModeExternal = false; // false=internal, true=external
    uint32_t buzzerTimer = 0;

    // runtime state for vibration
    bool vibrationModeExternal = false; // false=internal, true=external
    uint32_t vibrationTimer = 0;
};

extern Feedback openknxfeedback;