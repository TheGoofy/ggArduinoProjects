#pragma once

#include <Ticker.h>

class ggTimer {

public:

  ggTimer(float aTimeOutSeconds)
  : mTimeOutSeconds(aTimeOutSeconds),
    mTimeOutFunc(nullptr) {
    Reset();
  }

  void Reset() {
    mTicker.detach();
    mTicker.once(mTimeOutSeconds, [&] () {
      if (mTimeOutFunc != nullptr) mTimeOutFunc();
    });
  }

  typedef std::function<void()> tTimeOutFunc;
  
  void OnTimeOut(tTimeOutFunc aTimeOutFunc) {
    mTimeOutFunc = aTimeOutFunc;
  }

private:

  float mTimeOutSeconds;
  tTimeOutFunc mTimeOutFunc;
  Ticker mTicker;
  
};
