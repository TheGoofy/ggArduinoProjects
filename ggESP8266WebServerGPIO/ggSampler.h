#pragma once

class ggSampler {
  
public:
  
  typedef void (*tSampleFunction)();
  
  ggSampler(float aSamplesPerSecond,
            tSampleFunction aSampleFunction = nullptr)
  : mMicroSecondsSampleTime(1000000.0f / aSamplesPerSecond + 0.5f),
    mMicroSecondsLastSample(0),
    mMicroSecondsDelta(0),
    mSampleFunction(aSampleFunction) {
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

  void OnSample(tSampleFunction aSampleFunction) {
    mSampleFunction = aSampleFunction;
  }

  void Loop() {
    if (mSampleFunction != nullptr) {
      unsigned long vMicroSeconds = micros();
      unsigned long vMicroSecondsDelta = vMicroSeconds - mMicroSecondsLastSample;
      if (vMicroSecondsDelta >= mMicroSecondsSampleTime) {
        mMicroSecondsDelta = vMicroSecondsDelta;
        mMicroSecondsLastSample = vMicroSeconds;
        mSampleFunction();
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
  tSampleFunction mSampleFunction;
  
  mutable unsigned long mMicroSecondsLastSample;
  mutable unsigned long mMicroSecondsDelta;
  
};
