#pragma once

#include <Ticker.h>
#include <functional>
#include <vector>

class ggTicker {

public:

  typedef std::function<void(int aIntervalIndex)> tTickFunc;

  ggTicker(const std::vector<unsigned long>& aIntervals, bool aRepeat = true, unsigned long aTickDurationMillis = 100)
  : mIntervals(aIntervals),
    mRepeat(aRepeat),
    mTickDurationMillis(aTickDurationMillis),
    mTickFunc(nullptr),
    mIntervalIndex(0),
    mTickNext(0),
    mTick(0) {
  }

  unsigned long GetInterval(int aIntervalIndex) const {
    return mIntervals[aIntervalIndex];
  }

  void OnTick(tTickFunc aTickFunc) {
    mTickFunc = aTickFunc;
  }

  void Start() {
    mTicker.attach_ms(mTickDurationMillis, [&] () {
      Tick();
    });
  }

  void Stop() {
    mTicker.detach();
  }

  void Reset() {
    mIntervalIndex = 0;
    mTickNext = 0;
    mTick = 0;
  }

private:

  void Tick() {
    while (mTick >= mTickNext) {
      if (mTickFunc != nullptr) {
        mTickFunc(mIntervalIndex);
      }
      mTickNext += mIntervals[mIntervalIndex++];
      if (mIntervalIndex >= mIntervals.size()) {
        if (mRepeat) {
          mIntervalIndex = 0;
        }
        else {
          Stop();
          break;
        }
      }
    }
    mTick++;
  }

  Ticker mTicker;
  const unsigned long mTickDurationMillis;

  const std::vector<unsigned long> mIntervals;
  const bool mRepeat;

  tTickFunc mTickFunc;

  int mIntervalIndex;
  unsigned long mTick;
  unsigned long mTickNext;
  
};
