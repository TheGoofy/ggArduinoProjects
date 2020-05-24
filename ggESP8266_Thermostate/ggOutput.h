#pragma once

#include "ggInputOutput.h"
#include "ggDebug.h"

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

  bool GetInverted() const {
    return mInverted;
  }

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggOutput", aName);
    ggInputOutput::PrintDebug();
    vDebug.PrintF("mInverted = %d\n", mInverted);
    vDebug.PrintF("Get() = %d\n", Get());
  }

private:

  const bool mInverted;
  
};
