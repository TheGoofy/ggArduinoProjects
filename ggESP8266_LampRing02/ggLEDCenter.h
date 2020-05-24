#pragma once

#include "ggLog.h"
#include "ggValueEEPromT.h"
#include <Adafruit_PWMServoDriver.h>


class ggLEDCenter {

public:

  ggLEDCenter(int aPinSDA, int aPinSCL)
  : mPinSDA(aPinSDA),
    mPinSCL(aPinSCL),
    mLogTable(4096),
    mModulePWM(),
    mOn(false),
    mBrightness(0.5f) {
  }

  void Begin() {
    Wire.begin(mPinSDA, mPinSCL);
    mModulePWM.begin();
    mModulePWM.setOscillatorFrequency(28000000);  // The int.osc. is closer to 27MHz
    mModulePWM.setPWMFreq(300);  // 200 worls "silent", 1600 is the maximum PWM frequency
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

#define CHANNEL_R_WHITE_COLD  0
#define CHANNEL_R_ORANGE      2
#define CHANNEL_R_WHITE_WARM  4
#define CHANNEL_L_WHITE_COLD  6
#define CHANNEL_L_ORANGE      8
#define CHANNEL_L_WHITE_WARM 10

  void AnalogWrite(int aChannel, int aValue) {
    mModulePWM.setPin(aChannel, aValue);
  }

  void UpdateOutput() {
    mOutput = GetOn() ? mLogTable.Get(mBrightness.Get()) : 0;
    AnalogWrite(CHANNEL_L_WHITE_COLD, mOutput / 2);
    AnalogWrite(CHANNEL_L_WHITE_WARM, mOutput);
    AnalogWrite(CHANNEL_L_ORANGE, 0);
    AnalogWrite(CHANNEL_R_WHITE_COLD, mOutput);
    AnalogWrite(CHANNEL_R_WHITE_WARM, mOutput / 2);
    AnalogWrite(CHANNEL_R_ORANGE, 0);
  }

  // basic setup
  const int mPinSDA;
  const int mPinSCL;
  const ggLog mLogTable;
  Adafruit_PWMServoDriver mModulePWM;
  int mOutput;
  bool mOn;
  
  // persistent settings
  ggValueEEPromT<float> mBrightness;
  
};
