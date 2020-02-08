#pragma once

#include "ggOutput.h"

class ggOutputPWM : public ggOutput {

public:

  // maximum allowed cycle-time is approximately 30 minutes
  // (overvlow after 2^32 microseconds divided by 2)
  ggOutputPWM(int aPin,
              bool aInverted = false,
              float aCycleTimeSeconds = 1.0f)
  : ggOutput(aPin, aInverted),
    mCycleTime(1000000.0f * aCycleTimeSeconds + 0.5f),
    mCycleTimeHigh(0),
    mCycleTimeLow(0),
    mMicrosNext(0) {
    Set(0.0f);
  }

  void Set(bool aValue) {
    ggOutput::Set(aValue);
  }

  void Set(float aValue) {
    mCycleTimeHigh = static_cast<unsigned long>(aValue * mCycleTime + 0.5f);
    if (mCycleTimeHigh > mCycleTime) mCycleTimeHigh = mCycleTime;
    mCycleTimeLow = mCycleTime - mCycleTimeHigh;
  }

  float Get() const {
    return static_cast<float>(mCycleTimeHigh) / static_cast<float>(mCycleTime);
  }

  void Begin(float aValue = 0.0f) {
    Set(aValue);
    ggOutput::Begin(false);
    mMicrosNext = micros();
  }

  void Run() {
    unsigned long vMicros = micros();
    unsigned long vMicrosDelta = vMicros - mMicrosNext;
    // a "negative" UNSIGNED int32 is larger than 0x80000000 
    if (vMicrosDelta <= 0x80000000) {
      if (ggOutput::Get()) {
        if (mCycleTimeLow > 0) {
          ggOutput::Set(false);
          mMicrosNext += mCycleTimeLow;
          return;
        }
      }
      else {
        if (mCycleTimeHigh > 0) {
          ggOutput::Set(true);
          mMicrosNext += mCycleTimeHigh;
          return;
        }
      }
      mMicrosNext += mCycleTime;
    }
  }

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggOutputPWM", aName);
    ggOutput::PrintDebug();
    vDebug.PrintF("mCycleTime = %u\n", mCycleTime);
    vDebug.PrintF("mCycleTimeHigh = %u\n", mCycleTimeHigh);
    vDebug.PrintF("mCycleTimeLow = %u\n", mCycleTimeLow);
    vDebug.PrintF("mMicrosNext = %u\n", mMicrosNext);
    vDebug.PrintF("Run() ... micros() = %u\n", micros());
  }

private:

  unsigned long mCycleTime;
  unsigned long mCycleTimeHigh;
  unsigned long mCycleTimeLow;

  unsigned long mMicrosNext;
  
};
