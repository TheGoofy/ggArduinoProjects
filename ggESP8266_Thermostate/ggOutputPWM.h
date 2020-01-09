#pragma once

#include "ggOutput.h"

class ggOutputPWM  {

public:

  // maximum allowed cycle-time is approximately 30 minutes
  // (overvlow after 2^32 microseconds divided by 2)
  ggOutputPWM(int aPin,
              bool aInverted = false,
              float aCycleTimeSeconds = 1.0f)
  : mOutput(aPin, aInverted),
    mCycleTime(1000000.0f * aCycleTimeSeconds + 0.5f),
    mCycleTimeHigh(0),
    mCycleTimeLow(0),
    mMicrosNext(0) {
    Set(0.0f);
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
    mOutput.Begin(false);
    mMicrosNext = micros();
  }

  void Run() {
    unsigned long vMicros = micros();
    if (vMicros >= mMicrosNext) {
      unsigned long vMicrosDelta = vMicros - mMicrosNext;
      if (vMicrosDelta <= 0x8000) { // handle 32 bit overflow of mMicrosNext
        if (mOutput.Get()) {
          if (mCycleTimeLow > 0) {
            mOutput.Set(false);
            mMicrosNext += mCycleTimeLow;
            return;
          }
        }
        else {
          if (mCycleTimeHigh > 0) {
            mOutput.Set(true);
            mMicrosNext += mCycleTimeHigh;
            return;
          }
        }
        mMicrosNext += mCycleTime;
      }
    }
  }

private:

  ggOutput mOutput;

  unsigned long mCycleTime;
  unsigned long mCycleTimeHigh;
  unsigned long mCycleTimeLow;

  unsigned long mMicrosNext;
  
};
