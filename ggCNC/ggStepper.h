#ifndef __GG_STEPPER__
#define __GG_STEPPER__

#include <Arduino.h>

class ggStepper {

public:
  
  ggStepper(byte aEnablePin,
            byte aDirPin,
            byte aStepPin,
            byte aSensorPin)
  : mStepper(AccelStepper::DRIVER, aStepPin, aDirPin, 0, 0, false),
    mSensorPin(aSensorPin),
    mSpeedStepsPerSecond(100.0f),
    mUnitPerStep(1.0f)
  {
    mStepper.setPinsInverted(false, false, false);
    mStepper.setEnablePin(aEnablePin);
    mStepper.disableOutputs();
  }
  
  void begin()
  {
    pinMode(mSensorPin, INPUT);
  }
  
  void Enable()
  {
    mStepper.enableOutputs();
  }
  
  void Disable()
  {
    mStepper.disableOutputs();
  }
  
  void SetSpeedStep(float aStepsPerSecond)
  {
    mSpeedStepsPerSecond = aStepsPerSecond;
    mStepper.setMaxSpeed(aStepsPerSecond);
  }
  
  void SetAcceleration(float aAcceleration) // e.g. mm / s^2
  {
    mStepper.setAcceleration(aAcceleration / mUnitPerStep);
  }
  
  void SetSpeed(float aSpeed) // e.g. mm / s
  {
    SetSpeedStep(aSpeed / mUnitPerStep);
  }
  
  boolean Sensor() const
  {
    return digitalRead(mSensorPin) == LOW;
  }
  
  void InitPosition()
  {
    RunToSensor();
    mStepper.setCurrentPosition(0);
  }
  
  void MoveToPositionStep(long aPosition)
  {
    mStepper.moveTo(aPosition);
  }
  
  void RunToSensor()
  {
    if (Sensor()) {
      mStepper.setSpeed(mSpeedStepsPerSecond);
      while (Sensor()) mStepper.runSpeed();
      delay(100);
    }
    mStepper.setSpeed(-mSpeedStepsPerSecond);
    while (!Sensor()) mStepper.runSpeed();
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
    return mStepper.currentPosition() * mUnitPerStep;
  }
  
  boolean IsMoving() const
  {
    return mStepper.distanceToGo() != 0;
  }
  
  boolean run()
  {
    return mStepper.run();
  }

private:

  mutable AccelStepper mStepper;
  float mSpeedStepsPerSecond;
  float mUnitPerStep;
  byte mSensorPin;
  
};

/*
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
  
  void SetAcceleration(float aAcceleration) // e.g. mm / s^2
  {
    // mStepper.setAcceleration(aAcceleration / mUnitPerStep);
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
*/

#endif // __GG_STEPPER__
