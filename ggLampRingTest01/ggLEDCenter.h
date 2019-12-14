#pragma once

#include "ggLog.h"
#include "ggValueEEPromT.h"


template <class TValueType>
class ggValueT {

public:

  ggValueT(const TValueType& aValue)
  : mValue(aValue) {
  }

  const TValueType& Get() const {
    return mValue;
  }

  void Set(const TValueType& aValue) {
    mValue = aValue;
  }

private:

  TValueType mValue;
  
};


// #define ggValueT ggValueEEPromT


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
    float vBrightness = mBrightness.Get() + aBrightnessDelta;
    vBrightness = ggClamp(vBrightness, 0.0f, 1.0f);
    if (mBrightness.Get() != vBrightness) {
      mBrightness.Set(vBrightness);
      UpdateOutput();
    }
  }

private:

  void UpdateOutput() {
    Serial.printf("ggLEDCenter::UpdateOutput() - mOn=%d mBrightness=%f\n", mOn.Get(), mBrightness.Get());
    int vOutput = mOn.Get() ? mLogTable.Get(mBrightness.Get()) : 0;
    Serial.printf("ggLEDCenter::UpdateOutput() - vOutput=%d\n", vOutput);
    analogWrite(mPinPWM, vOutput);
  }

  // basic setup
  const ggLog mLogTable;
  const int mPinPWM;
  
  // persistent settings
  ggValueT<bool> mOn;
  ggValueT<float> mBrightness;
  
};
