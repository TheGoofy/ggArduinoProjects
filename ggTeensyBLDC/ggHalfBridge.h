#pragma once

class ggHalfBridge
{
  public:

    typedef void (*tSetupPWMFunc) (uint8_t aPin);
  
    ggHalfBridge(uint8_t aISPin, // current sense
                 uint8_t aENPin, // enable (inhibit)
                 uint8_t aPWMPin, // PWM duty cycle
                 tSetupPWMFunc aSetupPWMFunc,
                 uint16_t aPWMValueMax)
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

    void SetPWM(uint16_t aPWM) {
      mPWMValue = aPWM;
      analogWrite(mPWMPin, aPWM);
    }

    uint16_t GetPWM() const {
      return mPWMValue;
    }

  private:
  
    const uint8_t mISPin;
    const uint8_t mENPin;
    const uint8_t mPWMPin;
    const tSetupPWMFunc mSetupPWMFunc;
    const uint16_t mPWMValueMax;
    uint16_t mPWMValue;

};
