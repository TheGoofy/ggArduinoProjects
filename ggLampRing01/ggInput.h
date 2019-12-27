#pragma once

#include "ggInputOutput.h"

class ggInput : public ggInputOutput {

public:

  ggInput(int aPin,
          bool aInverted = false,
          bool aEnablePullUp = false)
  : ggInputOutput(aPin),
    mInverted(aInverted),
    mEnablePullUp(aEnablePullUp) {
  }

  void Begin() {
    pinMode(GetPin(), mEnablePullUp ? INPUT_PULLUP : INPUT);
  }

  bool Get() const {
    return digitalRead(GetPin()) ^ mInverted;
  }

private:

  const bool mInverted;
  const bool mEnablePullUp;
  
};
