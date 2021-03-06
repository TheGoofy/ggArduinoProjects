#ifndef __GG_BUTTON_2__
#define __GG_BUTTON_2__

#include <Arduino.h>

class ggButton2
{
  
public:

  ggButton2(byte aPin,
            boolean aInverted = false,
            boolean aEnablePullUp = false,
            int aMillisDelay = 25)
  : mPin(aPin),
    mInverted(aInverted),
    mEnablePullUp(aEnablePullUp),
    mMillisDelta(aMillisDelay),
    mMillis(0),
    mInput(false)
  {
  }
  
  void begin()
  {
    pinMode(mPin, INPUT);
    digitalWrite(mPin, mEnablePullUp);
  }
  
  boolean GetOn() const
  {
    return mInverted ^ mInput;
  }
  
  boolean GetOff() const
  {
    return mInverted ^ !mInput;
  }
  
  boolean Switching() const
  {
    boolean vInput = mInput;
    return vInput != ReadInput();
  }
  
  boolean SwitchingOn() const
  {
    return Switching() && GetOn();
  }
  
  boolean SwitchingOff() const
  {
    return Switching() && GetOff();
  }
  
private:

  boolean ReadInput() const
  {
    long vMillis = millis();
    if (vMillis - mMillis >= mMillisDelta) {
      boolean vInput = digitalRead(mPin);
      if (vInput != mInput) {
        mInput = vInput;
        mMillis = vMillis;
      }
    }
    return mInput;
  }
  
  const byte mPin;
  const boolean mInverted;
  const boolean mEnablePullUp;
  const long mMillisDelta;
  
  mutable long mMillis;
  mutable boolean mInput;
  
};

#endif // __GG_BUTTON_2__

