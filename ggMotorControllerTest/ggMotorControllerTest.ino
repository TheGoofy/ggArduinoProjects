#include <Servo.h> 
#include "ggSampler.h"
#include "ggBlinker.h"
#include "ggButton.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "ggLCD.h"
#include "ggQuadratureDecoder.h"


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
    pinMode(mPinPWM, INPUT);
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

  void SetPower(byte aPower)
  {
    if (mArmed) mServo.write(aPower);
  }

private:

  const byte mPinPWM;
  const byte mPinBEC;
  Servo mServo;
  long mActivationTime;
  boolean mActive;
  boolean mArmed;

};


ggMotorController mMotor(12, A1);
ggSampler mSampler(10.0f);
ggBlinker mBlinker(13);
ggButton mButton(A0, true, true);
ggLCD mLCD(0x27);
ggQuadratureDecoder mCounter(0x61);

void setup()
{
  mLCD.begin();
  mLCD.clear();
  mButton.begin();
  mMotor.begin();
  mBlinker.begin();
  mCounter.begin();
}


void loop()
{
  mMotor.run();
  mBlinker.run();
  
  if (mMotor.IsArmed()) {
    mBlinker.SetMillisOn(100);
    mBlinker.SetMillisOff(100);
    if (!mBlinker.IsBlinking()) mBlinker.BlinkStart();
  }
  else if (mMotor.IsActive()) {
    mBlinker.SetMillisOn(500);
    mBlinker.SetMillisOff(500);
    if (!mBlinker.IsBlinking()) mBlinker.BlinkStart();
  }
  else {
    mBlinker.BlinkStop();
  }
  
  static int vSpeed = 0;

  if (mButton.SwitchingOn()) {
    if (vSpeed < 0) vSpeed = 0;
    //else if (vSpeed < 25) vSpeed = 25;
    else if (vSpeed < 50) vSpeed = 50;
    //else if (vSpeed < 75) vSpeed = 75;
    else if (vSpeed < 100) vSpeed = 100;
    else vSpeed = 0;
    mLCD.PrintValue("Speed", vSpeed, "%", 0, 0);
    mMotor.SetPower(vSpeed);
    mCounter.Set(vSpeed);
  }
  
  int vSpeedNew = mCounter.Get();
  if (vSpeedNew != vSpeed) {
    if (vSpeedNew < 0) {
      vSpeedNew = 0;
      mCounter.Set(0);
    }
    if (vSpeedNew > 100) {
      vSpeedNew = 100;
      mCounter.Set(100);
    }
    vSpeed = vSpeedNew;
    mLCD.PrintValue("Speed", vSpeed, "%", 0, 0);
    mMotor.SetPower(vSpeed);    
  }

  /*
  if (mSampler.IsDue()) {
    int vValue = analogRead(A0);
    vValue = map(vValue, 0, 1023, 0, 180);
    mMotor.SetPower(vValue);
  }
  */
}
