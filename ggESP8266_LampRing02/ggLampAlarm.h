#pragma once

#include "ggAlarmClockNTP.h"

class ggLampAlarm : public ggAlarmClockNTP::cAlarm
{

public:

  ggLampAlarm()
  : mID(0),
    mSceneIndex(0),
    mDuration(0.0f) {
  }

  uint8_t mID;
  int8_t mSceneIndex;
  float mDuration;

  virtual String ToJson() const override {
    return ggAlarmClockNTP::cAlarm::ToJson() + ","
      "\"mID\":" + mID + ","
      "\"mSceneIndex\":" + mSceneIndex + ","
      "\"mDuration\":" + mDuration;
  }

};
