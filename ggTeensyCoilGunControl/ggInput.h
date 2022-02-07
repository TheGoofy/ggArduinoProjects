#pragma once

#include "ggInputOutput.h"
#include "ggDebug.h"

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

  bool GetInverted() const {
    return mInverted;
  }

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggInput", aName);
    ggInputOutput::PrintDebug();
    vDebug.PrintF("mInverted = %s\n", mInverted ? "true" : "false");
    vDebug.PrintF("mEnablePullUp = %s\n", mEnablePullUp ? "true" : "false");
    vDebug.PrintF("Get() = %s\n", Get() ? "true" : "false");
  }

private:

  const bool mInverted;
  const bool mEnablePullUp;
  
};
