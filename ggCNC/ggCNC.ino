#include <Wire.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <AccelStepper.h>
#include "ggLCD.h"
#include "ggQuadratureDecoder.h"
#include "ggStepper.h"
#include "ggMotorController.h"
#include "ggButton2.h"

// **********************************************************************************
// Motion control points

// perimeter: 150 mm
// number of holes: 60
// drill diameter: 5 mm
// gap between holes: 2.85 mm

const int mRotations = 1;
const int mNumberOfHoles = 60; // # count
const int mHoleIndexBegin = 0;
const int mHoleIndexEnd = mNumberOfHoles;

const float mAnglePositionHome = 0.0f; // degrees
const float mAnglePositionDelta = 360.0f / mNumberOfHoles; // degrees
const float mAngleSpeedInit = 30.0f; // degrees per second
const float mAngleSpeed = 180.0f; // degrees per second
const float mAngleAcceleration = 720.0f; // degrees per second^2

const float mRadiusPositionHome = 40.0f; // mm
const float mRadiusPositionDrillStart = 56.0f; // mm
const float mRadiusPositionDrillStop = mRadiusPositionDrillStart + 0.25f; // mm
const float mRadiusPositionDelta = 0.5f; // mm
const float mRadiusSpeedInit = 5.0f; // mm per second
const float mRadiusSpeedDrilling = 0.1f; // 1/3 * 0.6 mm per second for stainless steel
const float mRadiusSpeed = 13.0f; // mm per second
const float mRadiusAcceleration = 100.0f; // mm per second^2

const int mDrillSpeedOff = 0;
const int mDrillSpeedLow = 15;
const int mDrillSpeedHigh = 20;

// **********************************************************************************
// Motors

#define GG_STEPPER_ENABLE_PIN 8

#define GG_STEPPER_ANGLE_STEP_PIN 2
#define GG_STEPPER_ANGLE_DIR_PIN 5
#define GG_STEPPER_ANGLE_SENSOR_PIN 9

ggStepper mMotorAngle(GG_STEPPER_ENABLE_PIN,
                      GG_STEPPER_ANGLE_DIR_PIN,
                      GG_STEPPER_ANGLE_STEP_PIN,
                      GG_STEPPER_ANGLE_SENSOR_PIN);
                  
const long mMotorAngleStepsPerRevolution = 4L * 200L * 128L / 16L;
const float mMotorAngleDegreesPerStep = 360.0f / mMotorAngleStepsPerRevolution;

#define GG_STEPPER_X_STEP_PIN 3
#define GG_STEPPER_X_DIR_PIN 6
#define GG_STEPPER_X_SENSOR_PIN 10

ggStepper mMotorX(GG_STEPPER_ENABLE_PIN,
                  GG_STEPPER_X_DIR_PIN,
                  GG_STEPPER_X_STEP_PIN,
                  GG_STEPPER_X_SENSOR_PIN);

const long mMotorXStepsPerRevolution = 2L * 200L;
const float mMotorXMMPerStep = 1.5f / mMotorXStepsPerRevolution;

#define GG_MOTOR_DRILL_POWER_PIN 12
#define GG_MOTOR_DRILL_BEC_PIN A1

ggMotorController mMotorDrill(GG_MOTOR_DRILL_POWER_PIN,
                              GG_MOTOR_DRILL_BEC_PIN);

ggQuadratureDecoder mSensorAngle(0x60);
ggQuadratureDecoder mCounter1(0x61);

const long mSensorAngleStepsPerRevolution = 4L * 400L * 128L / 16L;
const float mSensorAngleDegreesPerStep = 360.0f / mSensorAngleStepsPerRevolution;

float GetSensorAngle()
{
  return mSensorAngle.Get() * mSensorAngleDegreesPerStep;
}

float GetMotorAngle()
{
  return mMotorAngle.GetPosition();
}

// **********************************************************************************
// UI: Displays & LEDs & Buttons ...

ggLCD mLCD(0x26);

ggButton2 mButton(A0, true, true);

void PrintMessage(const char* aMessage, int aRow = 0)
{
  mLCD.setCursor(0, aRow);
  mLCD.print(aMessage);
  mLCD.clearLineEnd();
}

void PrintValue(const char* aName, int aValue, int aRow = 0)
{
  mLCD.setCursor(0, aRow);
  mLCD.print(aName);
  mLCD.print(": ");
  mLCD.print(aValue);
  mLCD.clearLineEnd();
}

void PrintValue(const char* aName, float aValue, byte aDecimals = 3, int aRow = 0)
{
  mLCD.setCursor(0, aRow);
  mLCD.print(aName);
  mLCD.print(": ");
  mLCD.print(aValue, aDecimals);
  mLCD.clearLineEnd();
}

void PrintStatus(const char* aMessage,
                 int aHoleIndex)
{
  PrintMessage(aMessage, 0);
  PrintValue("Hole", aHoleIndex + 1, 1);
  PrintValue("Angle Set", GetMotorAngle(), 1, 2);
  PrintValue("Angle Act", GetSensorAngle(), 1, 3);
}

void PanicStop(const char* aMessage)
{
  PrintMessage(aMessage);
  mMotorAngle.Disable();
  mMotorX.Disable();
  mMotorDrill.SetSpeed(mDrillSpeedOff);
  while (true);
}

void CheckAngle(float aAngleErrorMax)
{
  float vAngleError = GetSensorAngle() - mMotorAngle.GetPosition();
  if (abs(vAngleError) > aAngleErrorMax) {
    PanicStop("Angle Error!");
  }
}

// **********************************************************************************
// Initialisation

typedef enum tPosition {
  ePositionUndefined,
  ePositionInit,
  ePositionStart,
  ePositionDrillStart,
  ePositionDrillStop,
  ePositionStop
};

tPosition mPosition = ePositionUndefined;

void setup()
{
  mLCD.begin();
  mLCD.clear();
  
  PrintMessage("Init ...");  

  mButton.begin();

  PrintMessage("Click to init!");  
  
  while (!mButton.SwitchingOn());
  
  mSensorAngle.begin();

  PrintMessage("Init X ...");  

  mMotorX.begin();
  mMotorX.SetUnitPerStep(mMotorXMMPerStep);
  mMotorX.SetAcceleration(mRadiusAcceleration);
  mMotorX.SetSpeed(mRadiusSpeedInit);
  mMotorX.Enable();
  mMotorX.InitPosition();

  PrintMessage("Init Angle ...");  

  mMotorAngle.begin();
  mMotorAngle.SetUnitPerStep(mMotorAngleDegreesPerStep);
  mMotorAngle.SetAcceleration(mAngleAcceleration);
  mMotorAngle.SetSpeed(mAngleSpeedInit);
  mMotorAngle.Enable();
  mMotorAngle.InitPosition();
  delay(200); // wait a wgile until system stops oscillations ...
  mSensorAngle.Set(0);
  
  PrintMessage("Power on Drill!", 0);
  PrintMessage("Click to continue.", 1);

  mMotorDrill.begin();

  while (!mButton.SwitchingOn());

  mLCD.clear();

  PrintMessage("Init Drill ...");  

  while (!mMotorDrill.IsArmed()) mMotorDrill.run();
  mMotorDrill.SetSpeed(0);
  mPosition = ePositionInit;

  PrintMessage("Click to Start!");  
  
  while (!mButton.SwitchingOn());
}

// **********************************************************************************
// Main program

boolean mRun = true;
int mHoleIndex = mHoleIndexBegin;
int mRotationIndex = 0;

void loop()
{ 
  if (!mMotorAngle.IsMoving() &&
      !mMotorX.IsMoving()) {
    
    // move from init- or stop-position to next start-position
    if ((mPosition == ePositionInit) ||
        (mPosition == ePositionStop)) {
      if (mHoleIndex >= mHoleIndexEnd) {
        mRotationIndex++;
        if (mRotationIndex < mRotations) {
          mHoleIndex = mHoleIndexBegin;
        }
      }    
      if (mHoleIndex < mHoleIndexEnd) {
        PrintStatus("Change Angle", mHoleIndex);
        mMotorAngle.SetSpeed(mAngleSpeed);
        mMotorAngle.MoveToPosition(mAnglePositionHome + mHoleIndex*mAnglePositionDelta);
        mMotorX.SetSpeed(mRadiusSpeed);
        mMotorX.MoveToPosition(mRadiusPositionHome);
        mMotorDrill.SetSpeed(mDrillSpeedLow);
        mPosition = ePositionStart;
      }
      else {
        PrintStatus("Finished", mHoleIndex - 1);
        mMotorAngle.Disable();
        mMotorX.Disable();
        mMotorDrill.SetSpeed(mDrillSpeedOff);
        mPosition = ePositionUndefined;
      }
    }

    // move from start-position to drill-start-position
    else if (mPosition == ePositionStart) {
      PrintStatus("Move Drill down", mHoleIndex);
      CheckAngle(0.6f);
      mMotorX.SetSpeed(mRadiusSpeed);
      mMotorX.MoveToPosition(mRadiusPositionDrillStart + mRotationIndex*mRadiusPositionDelta);
      mMotorDrill.SetSpeed(mDrillSpeedLow);
      mPosition = ePositionDrillStart;
    }
    
    // move from drill-start-position to drill-stop-position
    else if (mPosition == ePositionDrillStart) {
      PrintStatus("Drilling", mHoleIndex);
      CheckAngle(0.5f);
      mMotorX.SetSpeed(mRadiusSpeedDrilling);
      mMotorX.MoveToPosition(mRadiusPositionDrillStop + mRotationIndex*mRadiusPositionDelta);
      mMotorDrill.SetSpeed(mDrillSpeedHigh);
      mPosition = ePositionDrillStop;
    }
    
    // move from drill-stop-position to stop-position
    else if (mPosition == ePositionDrillStop) {
      PrintStatus("Move Drill up", mHoleIndex);
      mMotorX.SetSpeed(mRadiusSpeed);
      mMotorX.MoveToPosition(mRadiusPositionHome);
      mMotorDrill.SetSpeed(mDrillSpeedLow);
      mHoleIndex++; // hole finished - go to next hole
      mPosition = ePositionStop;
    }
  }
  
  if (mButton.SwitchingOn()) {
    mRun = !mRun;
    PrintMessage(mRun ? "Running" : "Pause");
    if (!mRun) mMotorDrill.SetSpeed(0);  
  }
    
  if (mRun) {
    mMotorAngle.run();
    mMotorX.run();
    mMotorDrill.run();
  }

}

