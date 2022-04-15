#pragma once

#include "ggTransitionT.h"


class ggLEDCenter {

public:

  ggLEDCenter(int aPinPWM)
  : mPinPWM(aPinPWM),
    mBrightness()
  {
    SetBrightness(0.0f, 0.3f); // transition-time
  }

  void Begin() {
    pinMode(mPinPWM, OUTPUT);
    analogWriteFreq(20000);
    UpdateOutput();
  }

  inline float GetBrightness() const {
    return mBrightness.Get();
  }

  void SetBrightness(float aBrightness, float aTransitionTime) {
    float vBrightness = ggClamp(aBrightness, 0.0f, 1.0f);
    if (mBrightness != vBrightness) {
      mBrightness.Set(vBrightness, aTransitionTime);
      UpdateOutput();
    }
  }

  inline void Stop() {
    analogWrite(mPinPWM, 0);
  }

  inline void Resume() {
    analogWrite(mPinPWM, mOutput);
  }

  void Run() {
    if (!mBrightness.Finished()) {
      UpdateOutput();
    }
  }

private:

  void UpdateOutput() {
    mOutput = ggRound<int>(1023.0f * mBrightness.Get());
    analogWrite(mPinPWM, mOutput);
  }

  // basic setup
  const int mPinPWM;
  int mOutput;
  ggTransitionT<float> mBrightness;

};
