#include "esp32-hal-gpio.h"
#pragma once

#include <AccelStepper.h>

class ggStepperAxis {

public:
  
  ggStepperAxis(byte aEnablePin,
                byte aDirPin,
                byte aStepPin,
                byte aSensorPin)
  : mStepper(AccelStepper::DRIVER, aStepPin, aDirPin, 0, 0, false),
    mSensorPin(aSensorPin),
    mUnitPerStep(1.0f)
  {
    mStepper.setPinsInverted(false, false, true);
    mStepper.setEnablePin(aEnablePin);
  }
  
  void Begin() {
    pinMode(mSensorPin, INPUT_PULLUP);
    mStepper.disableOutputs();
  }
  
  void Enable() {
    mStepper.enableOutputs();
  }
  
  void Disable() {
    mStepper.disableOutputs();
  }
  
  void SetSpeedStep(float aStepsPerSecond) {
    mStepper.setMaxSpeed(aStepsPerSecond);
    mStepper.setSpeed(aStepsPerSecond);
  }
  
  void SetAcceleration(float aAcceleration) {
    mStepper.setAcceleration(UnitToSteps(aAcceleration));
  }
  
  void SetSpeed(float aSpeed) {
    SetSpeedStep(UnitToSteps(aSpeed));
  }
  
  bool GetSensor() const {
    return digitalRead(mSensorPin) == HIGH;
  }

  void SetPosition(float aPosition) {
    mStepper.setCurrentPosition(lround(UnitToSteps(aPosition)));
  }
  
  float GetPosition() const {
    return StepsToUnit(mStepper.currentPosition());
  }
  
  float InitPosition(float aSensorPosition, float aSpeed) {
    MoveToSensor(aSpeed);
    float vOldPosition = GetPosition();
    SetPosition(aSensorPosition);
    return vOldPosition;
  }
  
  void MoveToPositionStep(long aPosition) {
    mStepper.moveTo(aPosition);
  }
  
  void MoveToSensor(float aSpeed) {
    float vSpeedStepsPerSecond = UnitToSteps(aSpeed);
    float vSpeedBackup = mStepper.speed();
    if (GetSensor()) {
      mStepper.setSpeed(vSpeedStepsPerSecond);
      while (GetSensor()) mStepper.runSpeed();
      delay(100);
    }
    mStepper.setSpeed(-vSpeedStepsPerSecond);
    while (!GetSensor()) mStepper.runSpeed();
    mStepper.setSpeed(vSpeedBackup);
  }
  
  void SetUnitPerStep(float aUnitPerStep) {
    mUnitPerStep = aUnitPerStep;
  }
  
  float GetUnitPerStep() const {
    return mUnitPerStep;
  }
  
  void MoveToPosition(float aPosition) {
    MoveToPositionStep(lround(UnitToSteps(aPosition)));
  }
  
  bool IsMoving() const {
    return mStepper.isRunning();
  }
  
  bool Run() {
    return mStepper.run();
  }

protected:

  inline float StepsToUnit(float aSteps) const {
    return aSteps * mUnitPerStep;
  }

  inline float UnitToSteps(float aUnit) const {
    return aUnit / mUnitPerStep;
  }

  mutable AccelStepper mStepper;

  float mUnitPerStep;
  const byte mSensorPin;
  
};
