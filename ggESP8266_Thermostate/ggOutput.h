#pragma once

#include "ggInputOutput.h"

class ggOutput : public ggInputOutput {

public:

  ggOutput(int aPin, bool aInv = false)
  : ggInputOutput(aPin),
    mInv(aInv) {
  }

  void Begin() {
    pinMode(GetPin(), OUTPUT);
  }

  void Begin(bool aValue) {
    Begin();
    Set(aValue);
  }

  bool Get() const {
    return digitalRead(GetPin()) ^ mInv;
  }

  void Set(bool aValue) {
    digitalWrite(GetPin(), aValue ^ mInv);
  }

private:

  bool mInv;
  
};

