#pragma once

#include <Ticker.h>
#include <functional>
#include <vector>

class ggTicker {

public:

  typedef std::vector<int> tIntervals;
  typedef std::function<void(int aIntervalIndex)> tTickFunc;

  ggTicker(const tIntervals& aIntervals = tIntervals(),
           int aTickDurationMillis = 100,
           bool aRepeat = true)
  : mTickDurationMillis(aTickDurationMillis),
    mIntervals(aIntervals),
    mRepeat(aRepeat),
    mTickFunc(nullptr),
    mTick(0),
    mTickNext(0),
    mIntervalIndex(0) {
  }

  void SetIntervals(const tIntervals& aIntervals) {
    bool vIsRunning = IsRunning();
    Stop();
    Reset();
    mIntervals = aIntervals;
    if (vIsRunning) Start();
  }

  int GetInterval(int aIntervalIndex) const {
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
    mTick = 0;
    mTickNext = 0;
    mIntervalIndex = 0;
  }

  bool IsRunning() const {
    return mTicker.active();
  }

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggTicker", aName);
    vDebug.PrintF("IsRunning() = %s\n", IsRunning() ? "true" : "false");
    vDebug.PrintF("mTickDurationMillis = %u\n", mTickDurationMillis);
    vDebug.PrintF("mRepeat = %s\n", mRepeat ? "true" : "false");
    vDebug.PrintF("mTick = %d\n", mTick);
    vDebug.PrintF("mTickNext = %d\n", mTickNext);
    vDebug.PrintF("mIntervalIndex = %d\n", mIntervalIndex);
    for (int vIntervalIndex = 0; vIntervalIndex < mIntervals.size(); vIntervalIndex++) {
      vDebug.PrintF("mIntervals[%d] = %d\n", vIntervalIndex, mIntervals[vIntervalIndex]);
    }
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

  // internal ticker
  Ticker mTicker;

  // settings
  const int mTickDurationMillis;
  tIntervals mIntervals;
  const bool mRepeat;

  // callback
  tTickFunc mTickFunc;

  // internal tick and interval states
  int mTick;
  int mTickNext;
  int mIntervalIndex;
  
};
