#pragma once

#include "ggIOPin.h"

class ggOutput : public ggIOPin {

public:

  ggOutput(int aPin,
           bool aInverted = false)
  : ggIOPin(aPin),
    mInverted(aInverted) {
  }

  void Begin() {
    pinMode(GetPin(), OUTPUT);
  }

  void Begin(bool aValue) {
    Begin();
    Set(aValue);
  }

  inline bool Get() const {
    return digitalRead(GetPin()) ^ mInverted;
  }

  inline void Set(bool aValue) {
    digitalWrite(GetPin(), aValue ^ mInverted);
  }

  inline bool GetInverted() const {
    return mInverted;
  }

private:

  const bool mInverted;
  
};
