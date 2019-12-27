#pragma once

#include <functional>

class ggSampler {
  
public:
  
  typedef std::function<void()> tSampleFunc;
  
  ggSampler(float aSamplesPerSecond,
            tSampleFunc aSampleFunc = nullptr)
  : mMicroSecondsSampleTime(1000000.0f / aSamplesPerSecond + 0.5f),
    mMicroSecondsLastSample(0),
    mMicroSecondsDelta(0),
    mSampleFunc(aSampleFunc) {
  }

  void SetSampleFrequency(float aSamplesPerSecond) {
    mMicroSecondsSampleTime = 1000000.0f / aSamplesPerSecond + 0.5f;
  }

  float GetSampleFrequency() const {
    return 1000000.0f / mMicroSecondsSampleTime;
  }

  void SetSamplePeriod(float aSampleTimeSeconds) {
    mMicroSecondsSampleTime = 1000000.0f * aSampleTimeSeconds + 0.5f;
  }

  float GetSamplePeriod() const {
    return 0.000001f * mMicroSecondsSampleTime;
  }

  void OnSample(tSampleFunc aSampleFunc) {
    mSampleFunc = aSampleFunc;
  }

  void Run() {
    if (mSampleFunc != nullptr) {
      unsigned long vMicroSeconds = micros();
      unsigned long vMicroSecondsDelta = vMicroSeconds - mMicroSecondsLastSample;
      if (vMicroSecondsDelta >= mMicroSecondsSampleTime) {
        mMicroSecondsDelta = vMicroSecondsDelta;
        mMicroSecondsLastSample = vMicroSeconds;
        mSampleFunc();
      }
    }
  }

  unsigned long GetLastSampleDeltaMicroSeconds() const {
    return mMicroSecondsDelta;
  }

  float GetLastSampleDeltaSeconds() const {
    return 0.000001f * mMicroSecondsDelta;
  }
  
private:
  
  unsigned long mMicroSecondsSampleTime;
  tSampleFunc mSampleFunc;
  
  mutable unsigned long mMicroSecondsLastSample;
  mutable unsigned long mMicroSecondsDelta;
  
};
