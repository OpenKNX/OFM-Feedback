#pragma once
#include "OpenKNX.h"

class Feedback : public OpenKNX::Module
{
  enum BuzzerVolume
  {
      BuzzerOff,    
      BuzzerSilent, 
      BuzzerNormal, 
      BuzzerLoud   
  };

  public:
    Feedback() {};

    const std::string version() override { return MODULE_Feedback_Version; }
    virtual const std::string name() override { return "Feedback"; };
    void processInputKo(GroupObject &iKo) override;
    virtual void setup() override;
    virtual void loop() override;
    void setBuzzer();
    void setVibration();
};

extern Feedback openknxfeedback;