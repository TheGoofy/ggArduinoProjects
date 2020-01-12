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

  bool GetInverted() const {
    return mInverted;
  }

  void Print(Stream& aStream) const {
    ggInputOutput::Print(aStream);
    aStream.printf("ggOutput::mInverted = %d\n", mInverted);
    aStream.printf("ggOutput::Get() = %d\n", Get());
  }

private:

  const bool mInverted;
  
};
