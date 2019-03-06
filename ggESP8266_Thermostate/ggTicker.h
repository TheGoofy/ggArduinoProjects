#pragma once

#include <functional>
#include <vector>

class ggTicker {

public:

  typedef std::function<void(int aIntervalIndex)> tTickFunc;

  ggTicker(const std::vector<unsigned long>& aIntervals, bool aRepeat = true)
  : mIntervals(aIntervals),
    mRepeat(aRepeat),
    mTickFunc(nullptr),
    mRunning(false),
    mIntervalIndex(0),
    mMillisNext(millis()) {
  }

  unsigned long GetInterval(int aIntervalIndex) const {
    return mIntervals[aIntervalIndex];
  }

  void OnTick(tTickFunc aTickFunc) {
    mTickFunc = aTickFunc;
  }

  void Start() {
    mRunning = true;
  }

  void Stop() {
    mRunning = false;
  }

  void Reset() {
    mIntervalIndex = 0;
    mMillisNext = millis();
  }

  void Run() {
    if (mRunning) {
      unsigned long vMillis = millis();
      if (vMillis >= mMillisNext) {
        if (mTickFunc != nullptr) {
          mTickFunc(mIntervalIndex);
        }
        mMillisNext += mIntervals[mIntervalIndex];
        mIntervalIndex++;
        if (mIntervalIndex >= mIntervals.size()) {
          if (mRepeat) {
            mIntervalIndex = 0;
          }
          else {
            mRunning = false;
          }
        }
      }
    }
  }

private:

  const std::vector<unsigned long> mIntervals;
  const bool mRepeat;

  tTickFunc mTickFunc;

  bool mRunning;
  int mIntervalIndex;
  unsigned long mMillisNext;
  
};
