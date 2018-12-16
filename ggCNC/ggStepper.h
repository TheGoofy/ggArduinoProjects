#ifndef __GG_STEPPER__
#define __GG_STEPPER__

#include <Arduino.h>


class ggStepper {

public:
  
  ggStepper(byte aEnablePin,
            byte aDirPin,
            byte aStepPin,
            byte aSensorPin)
  : mEnablePin(aEnablePin),
    mDirPin(aDirPin),
    mStepPin(aStepPin),
    mSensorPin(aSensorPin),
    mPosition(0),
    mPositionDest(0),
    mStep(0),
    mMicrosDelta(200),
    mUnitPerStep(1.0f)
  {
  }
  
  void begin()
  {
    pinMode(mEnablePin, OUTPUT);
    pinMode(mDirPin, OUTPUT);
    pinMode(mStepPin, OUTPUT);
    pinMode(mSensorPin, INPUT);
  }
  
  void Enable()
  {
    digitalWrite(mEnablePin, HIGH);
  }
  
  void Disable()
  {
    digitalWrite(mEnablePin, LOW);
  }
  
  void SetSpeedStep(long aStepsPerSecond)
  { 
    mMicrosDelta = 1000000L / aStepsPerSecond / 2;
  }
  
  void SetSpeed(float aSpeed) // e.g. mm / s
  {
    SetSpeedStep(aSpeed / mUnitPerStep);
  }
  
  boolean Sensor() const
  {
    return digitalRead(mSensorPin) == LOW;
  }
  
  void MoveDirUp()
  {
    digitalWrite(mDirPin, true);
    mStep = 1;
  }
  
  void MoveDirDown()
  {
    digitalWrite(mDirPin, false);
    mStep = -1;
  }
  
  void MoveDirStop()
  {
    mStep = 0;
  }
  
  void MoveStep()
  {
    delayMicroseconds(mMicrosDelta);
    digitalWrite(mStepPin, mPosition & 0x0001);
    mPosition += mStep;
    delayMicroseconds(mMicrosDelta);
    digitalWrite(mStepPin, mPosition & 0x0001);
    mPosition += mStep;
  }
  
  void InitPosition()
  {
    MoveToSensor();
    mPosition = 0;
    mPositionDest = 0;
    mMicros = micros();
  }
  
  void MoveToPositionStep(long aPosition)
  {
    mPositionDest = 2 * aPosition;
    if (mPositionDest > mPosition) MoveDirUp();
    if (mPositionDest < mPosition) MoveDirDown();
    if (mPositionDest == mPosition) MoveDirStop();
  }
  
  void MoveToSensor()
  {
    if (Sensor()) {
      MoveDirUp();
      while (Sensor()) MoveStep();
    }
    MoveDirDown();
    while (!Sensor()) MoveStep();
    MoveDirStop();
  }
  
  long GetPositionStep() const
  {
    return mPosition / 2;
  }
  
  void SetUnitPerStep(float aUnitPerStep)
  {
    mUnitPerStep = aUnitPerStep;
  }
  
  void MoveToPosition(float aPosition)
  {
    MoveToPositionStep(aPosition / mUnitPerStep);
  }
  
  float GetPosition() const
  {
    return GetPositionStep() * mUnitPerStep;
  }
  
  boolean IsMoving() const
  {
    return mStep != 0;
  }
  
  boolean run()
  {
    boolean vRun = mPosition != mPositionDest;
    if (vRun) {
      long vMicros = micros();
      long vMicrosDelta = vMicros - mMicros;
      if (vMicrosDelta >= mMicrosDelta) {
        digitalWrite(mStepPin, mPosition & 0x0001);
        mPosition += mStep;
        mMicros = vMicros;
      }
    }
    else {
      mStep = 0;
    }
    return vRun;
  }

private:

  byte mEnablePin;
  byte mDirPin;
  byte mStepPin;
  byte mSensorPin;
  long mPosition;
  long mPositionDest;
  int mStep;
  long mMicros;
  long mMicrosDelta;
  float mUnitPerStep;
  
};


#endif // __GG_STEPPER__
