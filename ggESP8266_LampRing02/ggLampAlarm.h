#pragma once

#include "ggAlarmClockNTP.h"

class ggLampAlarm : public ggAlarmClockNTP::cAlarm
{

public:

  uint8_t mScene;
  float mDuration;

  virtual String ToJson() const override {
    return ggAlarmClockNTP::cAlarm::ToJson() + ","
      "\"mScene\":" + mScene + ","
      "\"mDuration\":" + mDuration;
  }
  
};
