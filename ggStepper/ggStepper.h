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

  float GetSpeed() const
  {
    return mSpeedStepsPerSecond * mUnitPerStep;
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

  float GetUnitPerStep() const
  {
    return mUnitPerStep;
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

protected:

  mutable AccelStepper mStepper;

private:

  float mSpeedStepsPerSecond;
  float mUnitPerStep;
  byte mSensorPin;
  
};

#endif // __GG_STEPPER__