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

  private:
  
    const int mISPin;
    const int mENPin;
    const int mPWMPin;
    const tSetupPWMFunc mSetupPWMFunc;
    const int mPWMValueMax;
    int mPWMValue;

};
