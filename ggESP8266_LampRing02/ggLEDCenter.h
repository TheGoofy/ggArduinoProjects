#pragma once

#include "ggLog.h"
#include "ggValueEEPromT.h"
#include <Adafruit_PWMServoDriver.h>


template <int TNumChannels = 6>
class ggLEDCenter {

public:

  ggLEDCenter(int aPinSDA, int aPinSCL)
  : mPinSDA(aPinSDA),
    mPinSCL(aPinSCL),
    mLogTable(4095),
    mModulePWM(),
    mOn(false),
    mBrightness()
  {
    ResetSettings();
  }

  void Begin() {
    Wire.begin(mPinSDA, mPinSCL);
    mModulePWM.begin();
    mModulePWM.setOscillatorFrequency(28000000);  // The int.osc. is closer to 27MHz
    mModulePWM.setPWMFreq(180);  // 200 most "silent", 1600 is the maximum PWM frequency
    UpdateOutput();
  }

  void ResetSettings(float aChannelBrightness = 0.5f) {
    SetChannelBrightness(aChannelBrightness);
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
    ggValueEEProm::cLazyWriter vLazyWriter;
    ForEachChannel([&] (int aChannel) {
      const float vBrightness = mBrightness[aChannel] + aBrightnessDelta;
      mBrightness[aChannel] = ggClamp(vBrightness, 0.0f, 1.0f);;
    });
    UpdateOutput();
  }

  int GetNumberOfChannels() const {
    return TNumChannels;
  }

  typedef std::function<void(int aChannel)> tChannelFunc;
  void ForEachChannel(tChannelFunc aChannelFunc) const {
    for (int vChannel = 0; vChannel < TNumChannels; vChannel++) {
      aChannelFunc(vChannel);
    }
  }

  inline float GetChannelBrightness(int aChannel) const {
    if ((0 <= aChannel) && (aChannel < TNumChannels)) {
      return mBrightness[aChannel];
    }
    return 0.0f;
  }

  void SetChannelBrightness(float aBrightness) {
    ggValueEEProm::cLazyWriter vLazyWriter;
    ForEachChannel([&] (int aChannel) {
      mBrightness[aChannel] = aBrightness;
    });
    UpdateOutput();
  }

  void SetChannelBrightness(int aChannel, float aBrightness) {
    if ((0 <= aChannel) && (aChannel < TNumChannels)) {
      float vBrightness = ggClamp(aBrightness, 0.0f, 1.0f);
      if (mBrightness[aChannel] != vBrightness) {
        mBrightness[aChannel] = vBrightness;
        UpdateOutput(aChannel);
      }
    }
  }

private:

  inline static int GetChannelPWM(int aChannel) {
    switch (aChannel) {
      case 0: return  6; // right white cold
      case 1: return 10; // right white warm
      case 2: return  8; // right orange
      case 3: return  0; // left white cold
      case 4: return  4; // left white warm
      case 5: return  2; // left orange
      default: return 15; // last PWM-channel is unused
    }
  }

  inline void AnalogWrite(int aChannelPWM, int aValue) {
    if (aValue == 0) {
      mModulePWM.setPWM(aChannelPWM, 0, 4096);
    }
    else if (aValue >= 4095) {
      mModulePWM.setPWM(aChannelPWM, 4096, 0);
    }
    else {
      // stagger "on" in order to distribute current draw more evenly
      uint16_t vOn = aChannelPWM * 341; // same as 4096 / 12;
      uint16_t vOff = (vOn + aValue) & 0x0fff; // same as % 4096;
      mModulePWM.setPWM(aChannelPWM, vOn, vOff);
    }
  }

  inline void UpdateOutput(int aChannel) {
    int vChannelPWM = GetChannelPWM(aChannel);
    int vValuePWM = GetOn() ? mLogTable.Get(mBrightness[aChannel]) : 0;
    AnalogWrite(vChannelPWM, vValuePWM);
  }

  void UpdateOutput() {
    ForEachChannel([&] (int aChannel) {
      UpdateOutput(aChannel);
    });
  }

  // basic setup
  const int mPinSDA;
  const int mPinSCL;
  const ggLog mLogTable;
  Adafruit_PWMServoDriver mModulePWM;
  bool mOn;
  
  // persistent settings
  ggValueEEPromT<float> mBrightness[TNumChannels];
  
};
