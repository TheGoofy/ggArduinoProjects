#pragma once

#include "ggIOPin.h"

class ggInput : public ggIOPin {

public:

  ggInput(byte aPin,
          bool aInverted = false,
          bool aEnablePullUp = false)
  : ggIOPin(aPin),
    mInverted(aInverted),
    mEnablePullUp(aEnablePullUp) {
  }

  void Begin() {
    pinMode(GetPin(), mEnablePullUp ? INPUT_PULLUP : INPUT);
  }

  inline bool Get() const {
    return digitalRead(GetPin()) ^ mInverted;
  }

  inline bool GetInverted() const {
    return mInverted;
  }

private:

  const bool mInverted;
  const bool mEnablePullUp;
  
};
