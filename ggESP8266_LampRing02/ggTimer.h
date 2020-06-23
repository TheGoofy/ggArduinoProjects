#pragma once

class ggTimer {

public:

  ggTimer(float aTimeOutSeconds)
  : mTimeOutMicroSeconds(1000000.0f * aTimeOutSeconds + 0.5f),
    mLastMicroSeconds(-1), // "-1" indicates a stopped timer
    mTimeOutFunc(nullptr) {
  }

  void SetTimeOut(float aTimeOutSeconds) {
    mTimeOutMicroSeconds = 1000000.0f * aTimeOutSeconds + 0.5f;
  }

  float GetTimeOut() const {
    return 0.000001f * mTimeOutMicroSeconds;
  }

  void Start() {
    mLastMicroSeconds = micros();
    // slightly cheat if current time is "-1" by accident
    if (mLastMicroSeconds == -1) mLastMicroSeconds = 0;
  }

  void Stop() {
    mLastMicroSeconds = -1;
  }

  bool IsStarted() const {
    return mLastMicroSeconds != -1;
  }

  bool IsStopped() const {
    return mLastMicroSeconds == -1;
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

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggTimer", aName);
    vDebug.PrintF("mTimeOutMicroSeconds = %d\n", mTimeOutMicroSeconds);
    vDebug.PrintF("mLastMicroSeconds = %d\n", mLastMicroSeconds);
    vDebug.PrintF("micros() = %d\n", micros());
    vDebug.PrintF("mTimeOutFunc = 0x%08X\n", std::addressof(mTimeOutFunc));
  }

private:

  unsigned long mTimeOutMicroSeconds;
  unsigned long mLastMicroSeconds;
  tTimeOutFunc mTimeOutFunc;
  
};
