#pragma once

#include "ggLog.h"
#include "ggValueEEPromT.h"


class ggLEDCenter {

public:

  ggLEDCenter(int aPinPWM)
  : mLogTable(1023),
    mPinPWM(aPinPWM),
    mOn(false),
    mBrightness(0.5f) {
  }

  void Begin() {
    pinMode(mPinPWM, OUTPUT);
    analogWriteFreq(10000);
    UpdateOutput();
  }

  bool GetOn() const {
    return mOn.Get();
  }

  void SetOn(bool aOn) {
    if (mOn.Get() != aOn) {
      mOn.Set(aOn);
      UpdateOutput();
    }
  }

  void ToggleOnOff() {
    SetOn(!GetOn());
  }

  void ChangeBrightness(const float& aBrightnessDelta) {
    if (!GetOn()) return;
    float vBrightness = mBrightness.Get() + aBrightnessDelta;
    vBrightness = ggClamp(vBrightness, 0.0f, 1.0f);
    if (mBrightness.Get() != vBrightness) {
      mBrightness.Set(vBrightness);
      UpdateOutput();
    }
  }

private:

  void UpdateOutput() {
    int vOutput = mOn.Get() ? mLogTable.Get(mBrightness.Get()) : 0;
    analogWrite(mPinPWM, vOutput);
  }

  // basic setup
  const ggLog mLogTable;
  const int mPinPWM;
  
  // persistent settings
  ggValueEEPromT<bool> mOn;
  ggValueEEPromT<float> mBrightness;
  
};
