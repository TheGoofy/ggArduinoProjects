#ifndef __GG_MOTOR_CONTROLLER__
#define __GG_MOTOR_CONTROLLER__

#include <Arduino.h>


/**
 * When connecting the battery, power output of RC controllers is usually disabled, if the
 * throttle isn't in OFF-position. This prevents that the motor starts accidentially.
 * The controller only starts, if the throttle is constant at the minimum
 * for a while. This class uses the BEC as input signal in order to check, if the battery
 * is plugged. When the BEC-signal goes HIGH, it waits  a few seconds, until it's armed ...
 */
class ggMotorController
{
public:

  ggMotorController(byte aPinPWM,
                    byte aPinBEC)
  : mPinPWM(aPinPWM),
    mPinBEC(aPinBEC),
    mActivationTime(0),
    mActive(false),
    mArmed(false)
  {
  }
  
  void begin()
  {
    pinMode(mPinBEC, INPUT);
    mServo.attach(mPinPWM);
    mServo.write(0);
  }

  void run()
  {
    boolean vActive = digitalRead(mPinBEC);
    if (vActive != mActive) {
      mActive = vActive;
      mArmed = false;
      mServo.write(0);
      if (mActive) mActivationTime = millis();
    }
    if (mActive && !mArmed) {
      mArmed = millis() - mActivationTime > 3000;
    }
  }
  
  boolean IsActive() const
  {
    return mActive;
  }

  boolean IsArmed() const
  {
    return mArmed;
  }

  void SetSpeed(byte aSpeed)
  {
    if (mArmed) mServo.write(aSpeed);
  }
  
  byte GetSpeed() const
  {
    return mServo.read();
  }

private:

  const byte mPinPWM;
  const byte mPinBEC;
  mutable Servo mServo;
  long mActivationTime;
  boolean mActive;
  boolean mArmed;

};


#endif // __GG_MOTOR_CONTROLLER__

