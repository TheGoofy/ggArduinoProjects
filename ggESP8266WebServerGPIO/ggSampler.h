#pragma once

class ggSampler {
  
public:
  
  ggSampler(float aSamplesPerSecond)
  : mMicroSecondsSamplingTime(1000000.0f / aSamplesPerSecond),
    mMicroSecondsLastSample(0),
    mMicroSecondsDelta(0) {
  }

  bool IsDue() {
    unsigned long vMicroSeconds = micros();
    unsigned long vMicroSecondsDelta = vMicroSeconds - mMicroSecondsLastSample;
    if (vMicroSecondsDelta >= mMicroSecondsSamplingTime) {
      mMicroSecondsDelta = vMicroSecondsDelta;
      mMicroSecondsLastSample = vMicroSeconds;
      return true;
    }
    else {
      return false;
    }
  }

  unsigned long GetLastSampleDeltaMicroSeconds() const {
    return mMicroSecondsDelta;
  }

  float GetLastSampleDeltaSeconds() const {
    return 0.000001f * mMicroSecondsDelta;
  }
  
private:
  
  const unsigned long mMicroSecondsSamplingTime;
  unsigned long mMicroSecondsLastSample;
  unsigned long mMicroSecondsDelta;
  
};
