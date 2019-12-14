#pragma once

#include "ggInputOutput.h"

class ggOutput : public ggInputOutput {

public:

  ggOutput(int aPin,
           bool aInverted = false)
  : ggInputOutput(aPin),
    mInverted(aInverted) {
  }

  void Begin() {
    pinMode(GetPin(), OUTPUT);
  }

  void Begin(bool aValue) {
    Begin();
    Set(aValue);
  }

  bool Get() const {
    return digitalRead(GetPin()) ^ mInverted;
  }

  void Set(bool aValue) {
    digitalWrite(GetPin(), aValue ^ mInverted);
  }

private:

  const bool mInverted;
  
};

