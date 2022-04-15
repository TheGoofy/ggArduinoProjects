#pragma once

#include "ggAlarmClockNTP.h"

class ggLampAlarm : public ggAlarmClockNTP::cAlarm
{

public:

  ggLampAlarm()
  : mID(0),
    mSceneIndex(0),
    mTransitionTime(0.0f) {
  }

  uint8_t mID;
  int8_t mSceneIndex;
  float mTransitionTime;

  virtual String ToJson() const override {
    return ggAlarmClockNTP::cAlarm::ToJson() + ","
      "\"mID\":" + mID + ","
      "\"mSceneIndex\":" + mSceneIndex + ","
      "\"mTransitionTime\":" + mTransitionTime;
  }

};
