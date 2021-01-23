#pragma once

class ggFullBridge
{
  public:

    typedef void (*tSetupPWMFunc) (int aPin);

    ggFullBridge(int aPPin, // PWM signal
                 int a1Pin, // direction and brake
                 int a2Pin, // direction and brake
                 tSetupPWMFunc aSetupPWMFunc,
                 int aPWMValueMax)
    : mPPin(aPPin),
      m1Pin(a1Pin),
      m2Pin(a2Pin),
      mSetupPWMFunc(aSetupPWMFunc),
      mPWMValueMax(aPWMValueMax) {       
    }

    void Begin() {
      pinMode(mPPin, INPUT);
      pinMode(m1Pin, OUTPUT);
      pinMode(m2Pin, OUTPUT);
      mSetupPWMFunc(mPPin);
    }

    void SetBrake(bool aBrake) {
      digitalWrite(m1Pin, !aBrake);
      digitalWrite(m2Pin, !aBrake);
    }

    void SetDirection(bool aDirection) {
      digitalWrite(m1Pin, aDirection);
      digitalWrite(m2Pin, !aDirection);
    }

    void SetPWM(int aPWM) {
      analogWrite(mPPin, aPWM);
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
  
    const int mPPin;
    const int m1Pin;
    const int m2Pin;
    const tSetupPWMFunc mSetupPWMFunc;
    const int mPWMValueMax;

};
