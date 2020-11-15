#pragma once

#include "ggButton.h"

class ggKey : public ggButton
{
  
public:

  ggKey(byte aPin,
        boolean aInverted = false,
        boolean aEnablePullUp = false,
        float aDecaySeconds = 0.025f)
  : ggButton(aPin, aInverted, aEnablePullUp, aDecaySeconds)
  {
    SetRepeat(0.7f, 10.0f);
  }

  void SetRepeat(float aDelaySeconds,
                 float aRepeatRateHZ)
  {
    mRepeatDelayMillis = 1000.0f * aDelaySeconds;
    mRepeatIntervalMillis = 1000.0f / aRepeatRateHZ;
    mRepeatNextMillis = mRepeatDelayMillis;
  }

  boolean Pressed() const
  {
    boolean vPressed = SwitchingOn();

    long vTimeOn = GetTimeOn();
    if (vTimeOn < 0) mRepeatNextMillis = mRepeatDelayMillis;
    if (vTimeOn >= mRepeatNextMillis) {
      mRepeatNextMillis += mRepeatIntervalMillis;
      vPressed = true;
    }
    
    return vPressed;    
  }

private:

  long mRepeatDelayMillis;
  long mRepeatIntervalMillis;
  mutable long mRepeatNextMillis;
  
};
