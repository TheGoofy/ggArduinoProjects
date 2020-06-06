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
    mLogTable(4096),
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

  float GetBrightness() const {
    float vBrightness = 0.0f;
    ForEachChannel([&] (int aChannel) {
      vBrightness += GetChannelBrightness(aChannel);
    });
    return vBrightness / GetNumberOfChannels();
  }

  void SetBrightness(float aBrightness) {
    float vBrightnessOld = GetBrightness();
    if (vBrightnessOld == 0.0f) {
      SetChannelBrightness(aBrightness);
    }
    else if (!ggEqual(aBrightness, vBrightnessOld)) {
      float vRatio = aBrightness / vBrightnessOld;
      float vBrightness[TNumChannels];
      ForEachChannel([&] (int aChannel) {
        vBrightness[aChannel] = mBrightness[aChannel];
      });
      int vNumSaturatedChannels = 0;
      float vBrightnessOverflow = 0.0f;
      ForEachChannel([&] (int aChannel) {
        vBrightness[aChannel] *= vRatio;
        if (vBrightness[aChannel] > 1.0f) {
          vBrightnessOverflow += vBrightness[aChannel] - 1.0f;
          vBrightness[aChannel] = 1.0f;
        }
        if (vBrightness[aChannel] >= 1.0) {
          vNumSaturatedChannels++;
        }
      });
      while ((vBrightnessOverflow > 0.0f) &&
             (vNumSaturatedChannels < TNumChannels)) {
        float vBrightnessAdd = vBrightnessOverflow / (TNumChannels - vNumSaturatedChannels);
        vBrightnessOverflow = 0.0f;
        vNumSaturatedChannels = 0;
        ForEachChannel([&] (int aChannel) {
          if (vBrightness[aChannel] < 1.0f) {
            vBrightness[aChannel] += vBrightnessAdd;
            if (vBrightness[aChannel] > 1.0f) {
              vBrightnessOverflow += vBrightness[aChannel] - 1.0f;
              vBrightness[aChannel] = 1.0f;
            }
          }
          if (vBrightness[aChannel] >= 1.0) {
            vNumSaturatedChannels++;
          }
        });
      }
      ForEachChannel([&] (int aChannel) {
        mBrightness[aChannel] = vBrightness[aChannel];
      });
      UpdateOutput();
    }
  }

  void ChangeBrightness(float aBrightnessDelta) {
    SetBrightness(GetBrightness() + aBrightnessDelta);
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

  float GetChannelBrightness(int aChannel) const {
    if ((0 <= aChannel) && (aChannel < TNumChannels)) {
      return mBrightness[aChannel];
    }
    return 0.0;
  }

  void SetChannelBrightness(float aBrightness) {
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

  static int GetChannelPWM(int aChannel) {
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

  void AnalogWrite(int aChannelPWM, int aValue) {
    mModulePWM.setPin(aChannelPWM, aValue);
  }

  void UpdateOutput(int aChannel) {
    int vChannelPWM = GetChannelPWM(aChannel);
    int vValuePWM = GetOn() ? mLogTable.Get(mBrightness[aChannel].Get()) : 0;
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
