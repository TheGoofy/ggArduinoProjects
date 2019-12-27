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
    analogWriteFreq(20000);
    UpdateOutput();
  }

  bool GetOn() const {
    return mOn;
  }

  void SetOn(bool aOn) {
    if (mOn != aOn) {
      mOn = aOn;
      UpdateOutput();
    }
  }

  void ToggleOnOff() {
    SetOn(!GetOn());
  }

  void ChangeBrightness(const float& aBrightnessDelta) {
    if (!GetOn()) return;
    float vBrightness = mBrightness + aBrightnessDelta;
    vBrightness = ggClamp(vBrightness, 0.0f, 1.0f);
    if (mBrightness != vBrightness) {
      mBrightness = vBrightness;
      UpdateOutput();
    }
  }

  inline void Stop() {
    analogWrite(mPinPWM, 0);
  }

  inline void Resume() {
    analogWrite(mPinPWM, mOutput);
  }

private:

  void UpdateOutput() {
    mOutput = GetOn() ? mLogTable.Get(mBrightness.Get()) : 0;
    analogWrite(mPinPWM, mOutput);
  }

  // basic setup
  const ggLog mLogTable;
  const int mPinPWM;
  int mOutput;
  bool mOn;
  
  // persistent settings
  ggValueEEPromT<float> mBrightness;
  
};
