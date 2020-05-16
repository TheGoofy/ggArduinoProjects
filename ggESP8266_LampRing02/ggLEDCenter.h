#pragma once

#include "ggLog.h"
#include "ggValueEEPromT.h"
#include <Adafruit_PWMServoDriver.h>


class ggLEDCenter {

public:

  ggLEDCenter()
  : mLogTable(4096),
    mModulePWM(),
    mOn(false),
    mBrightness(0.5f) {
  }

  void Begin() {
    mModulePWM.begin();
    mModulePWM.setOscillatorFrequency(28000000);  // The int.osc. is closer to 27MHz
    mModulePWM.setPWMFreq(800);  // 200 worls "silent", 1600 is the maximum PWM frequency
    for (int vChannel = 0; vChannel < 16; vChannel++) AnalogWrite(vChannel, 0);
    UpdateOutput();
  }

  void ResetSettings() {
    SetBrightness(0.5f);
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

  float GetBrightness() const {
    return mBrightness;
  }

  void SetBrightness(const float aBrightness) {
    float vBrightness = ggClamp(aBrightness, 0.0f, 1.0f);
    if (mBrightness != vBrightness) {
      mBrightness = vBrightness;
      UpdateOutput();
    }
  }

  void ChangeBrightness(const float aBrightnessDelta) {
    SetBrightness(mBrightness + aBrightnessDelta);
  }

private:

  void AnalogWrite(int aChannel, int aValue) {
    mModulePWM.setPin(aChannel, aValue);
  }

  void UpdateOutput() {
    mOutput = GetOn() ? mLogTable.Get(mBrightness.Get()) : 0;
    AnalogWrite( 0, mOutput);
    AnalogWrite( 2, mOutput);
    AnalogWrite( 4, mOutput);
    AnalogWrite( 6, mOutput);
    AnalogWrite( 8, mOutput);
    AnalogWrite(10, mOutput);
  }

  // basic setup
  const ggLog mLogTable;
  Adafruit_PWMServoDriver mModulePWM;
  int mOutput;
  bool mOn;
  
  // persistent settings
  ggValueEEPromT<float> mBrightness;
  
};
