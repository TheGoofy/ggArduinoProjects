#pragma once

#include "ggInputOutput.h"

class ggInput : public ggInputOutput {

public:

  ggInput(int aPin, bool aInv = false)
  : ggInputOutput(aPin),
    mInv(aInv) {
  }

  void Begin() {
    pinMode(GetPin(), INPUT);
  }

  bool Get() const {
    return digitalRead(GetPin()) ^ mInv;
  }

private:

  bool mInv;
  
};

