#pragma once

#include "ggOutput.h"
#include "ggTicker.h"

class ggOutputPWM : public ggOutput {

public:

  // pwm resolution is 10 ms
  ggOutputPWM(int aPin,
              bool aInverted = false,
              float aCycleTimeSeconds = 1.0f)
  : ggOutput(aPin, aInverted),
    mCycles(100.0f * aCycleTimeSeconds + 0.5f), // 1[s] = 1000[ms] = 100 * TickDuration = 100 * 10[ms]
    mCyclesHigh(0),
    mCyclesLow(100),
    mTicker({0, 100}, 10, true) { // 0 high-ticks, 100 low-ticks, 10 ms per tick
    mTicker.OnTick([&] (int aIntervalIndex) {
      if (aIntervalIndex == 0) {
        if (mCyclesHigh > 0) ggOutput::Set(true);
      }
      else {
        if (mCyclesLow > 0) ggOutput::Set(false);
      }
    });
  }

  void Set(bool aValue) {
    mTicker.Stop();
    ggOutput::Set(aValue);
  }

  void Set(float aValue) {
    mCyclesHigh = static_cast<unsigned long>(aValue * mCycles + 0.5f);
    if (mCyclesHigh > mCycles) mCyclesHigh = mCycles;
    mCyclesLow = mCycles - mCyclesHigh;
    mTicker.SetIntervals({mCyclesHigh, mCyclesLow});
    mTicker.Start();
  }

  float Get() const {
    return static_cast<float>(mCyclesHigh) / static_cast<float>(mCycles);
  }

  void Begin(float aValue = 0.0f) {
    ggOutput::Begin(false);
    Set(aValue);
  }

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggOutputPWM", aName);
    ggOutput::PrintDebug();
    vDebug.PrintF("mCycles = %u\n", mCycles);
    vDebug.PrintF("mCyclesHigh = %u\n", mCyclesHigh);
    vDebug.PrintF("mCyclesLow = %u\n", mCyclesLow);
    mTicker.PrintDebug("mTicker");
  }

private:

  unsigned long mCycles;
  unsigned long mCyclesHigh;
  unsigned long mCyclesLow;

  ggTicker mTicker;
  
};
