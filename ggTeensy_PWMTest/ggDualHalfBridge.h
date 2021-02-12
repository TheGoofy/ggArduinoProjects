#pragma once

#include "ggHalfBridge.h"

class ggDualHalfBridge
{
  public:

    typedef void (*tSetupPWMFunc) (int aPin);

    ggDualHalfBridge(int aLISPin, int aLENPin, int aLPWMPin, // L: current sense, enable, PWM duty cycle
                     int aRISPin, int aRENPin, int aRPWMPin, // L: current sense, enable, PWM duty cycle
                     tSetupPWMFunc aSetupPWMFunc,
                     int aPWMValueMax)
    : mHalfBridgeL(aLISPin, aLENPin, aLPWMPin, aSetupPWMFunc, aPWMValueMax),
      mHalfBridgeR(aRISPin, aRENPin, aRPWMPin, aSetupPWMFunc, aPWMValueMax),
      mDirection(false),
      mPWM(0) {
    }

    void Begin() {
      mHalfBridgeL.Begin();
      mHalfBridgeR.Begin();
    }

    void SetBrake(bool aBrake) {
      mHalfBridgeL.SetEnable(aBrake);
      mHalfBridgeR.SetEnable(aBrake);
    }

    void SetDirection(bool aDirection) {
      if (mDirection != aDirection) {
        mDirection = aDirection;
        UpdatePWM();
      }
    }

    void SetPWM(int aPWM) {
      if (mPWM != aPWM) {
        mPWM = aPWM;
        UpdatePWM();
      }
    }

    void SweepUp(unsigned long aTimeMS = 5000) {
      HalfBridge().SweepUp(aTimeMS);
    }

    void SweepDown(unsigned long aTimeMS = 5000) {
      HalfBridge().SweepDown(aTimeMS);
    }
  
  private:

    void UpdatePWM() {
      digitalWrite(13, mDirection);
      mHalfBridgeL.SetPWM(mDirection ? mPWM : 0);
      mHalfBridgeR.SetPWM(mDirection ? 0 : mPWM);
    }

    ggHalfBridge& HalfBridge() {
      return mDirection ? mHalfBridgeL : mHalfBridgeR;
    }

    ggHalfBridge mHalfBridgeL;
    ggHalfBridge mHalfBridgeR;

    bool mDirection;
    int mPWM;
};
