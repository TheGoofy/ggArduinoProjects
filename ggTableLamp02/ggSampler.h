#pragma once

class ggSampler
{
  
public:
  
  ggSampler(float aSamplesPerSecond);
  
  bool IsDue();
  unsigned long GetLastSampleDeltaMicroSeconds() const;
  float GetLastSampleDeltaSeconds() const;
  
private:
  
  const unsigned long mMicroSecondsSamplingTime;
  unsigned long mMicroSecondsLastSample;
  unsigned long mMicroSecondsDelta;
  
};
