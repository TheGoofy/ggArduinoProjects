#pragma once

class ggTimer {

public:

  ggTimer(float aTimeOutSeconds)
  : mTimeOutMicroSeconds(1000000.0f * aTimeOutSeconds + 0.5f),
    mLastMicroSeconds(-1),
    mTimeOutFunc(nullptr) {
  }

  void Reset() {
    mLastMicroSeconds = micros();
    if (mLastMicroSeconds == -1) mLastMicroSeconds = 0;
  }

  typedef std::function<void()> tTimeOutFunc;
  
  void OnTimeOut(tTimeOutFunc aTimeOutFunc) {
    mTimeOutFunc = aTimeOutFunc;
  }

  void Run() {
    if (mTimeOutFunc != nullptr) {
      if (mLastMicroSeconds != -1) {
        unsigned long vMicroSeconds = micros();
        unsigned long vMicroSecondsDelta = vMicroSeconds - mLastMicroSeconds;
        if (vMicroSecondsDelta >= mTimeOutMicroSeconds) {
          mLastMicroSeconds = -1;
          mTimeOutFunc();
        }
      }
    }
  }

private:

  unsigned long mTimeOutMicroSeconds;
  unsigned long mLastMicroSeconds;
  tTimeOutFunc mTimeOutFunc;
  
};
