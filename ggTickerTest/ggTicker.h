#pragma once

#include <functional>
#include <vector>

class ggTicker {

public:

  typedef std::function<void(int aIntervalIndex)> tTickFunc;

  ggTicker(const std::vector<unsigned long>& aIntervals, bool aRepeat)
  : mIntervals(aIntervals),
    mRepeat(aRepeat),
    mTickFunc(nullptr),
    mRunning(false),
    mIntervalIndex(0),
    mMicrosNext(millis()) {
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
    mMicrosNext = millis();
  }

  void Run() {
    if (mRunning) {
      unsigned long vMicros = millis();
      if (vMicros >= mMicrosNext) {
        if (mTickFunc != nullptr) {
          mTickFunc(mIntervalIndex);
        }
        mMicrosNext += mIntervals[mIntervalIndex];
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
  unsigned long mMicrosNext;
  
};
