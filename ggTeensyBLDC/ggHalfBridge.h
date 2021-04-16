#pragma once

class ggHalfBridge
{
  public:

    typedef void (*tSetupPWMFunc) (int aPin);
  
    ggHalfBridge(int aISPin, // current sense
                 int aENPin, // enable (inhibit)
                 int aPWMPin, // PWM duty cycle
                 tSetupPWMFunc aSetupPWMFunc,
                 int aPWMValueMax)
    : mISPin(aISPin),
      mENPin(aENPin),
      mPWMPin(aPWMPin),
      mSetupPWMFunc(aSetupPWMFunc),
      mPWMValueMax(aPWMValueMax),
      mPWMValue(0) {
    }

    void Begin() {
      pinMode(mISPin, INPUT);
      pinMode(mENPin, OUTPUT);
      pinMode(mPWMPin, OUTPUT);
      mSetupPWMFunc(mPWMPin);
    }

    void SetEnable(bool aEnable) {
      digitalWrite(mENPin, aEnable);
    }

    void SetPWM(int aPWM) {
      mPWMValue = aPWM;
      analogWrite(mPWMPin, aPWM);
    }

    int GetPWM() const {
      return mPWMValue;
    }

    void SweepUp(unsigned long aTimeMS = 5000) {
      const unsigned long vDelayUS = 1000 * aTimeMS / mPWMValueMax;
      for (int vPWM = 0; vPWM <= mPWMValueMax; vPWM++) {
        SetPWM(vPWM);
        delayMicroseconds(vDelayUS);
      }
    }

    void SweepDown(unsigned long aTimeMS = 5000) {
      const unsigned long vDelayUS = 1000 * aTimeMS / mPWMValueMax;
      for (int vPWM = mPWMValueMax; vPWM >= 0; vPWM--) {
        SetPWM(vPWM);
        delayMicroseconds(vDelayUS);
      }
    }

  private:
  
    const int mISPin;
    const int mENPin;
    const int mPWMPin;
    const tSetupPWMFunc mSetupPWMFunc;
    const int mPWMValueMax;
    int mPWMValue;

};
