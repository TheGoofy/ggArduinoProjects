#include <Wire.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <AccelStepper.h>
#include <EEPROM.h>
#include "ggLCD.h"
#include "ggStepper.h"
#include "ggMotorController.h"
#include "ggButtonSimple.h"
#include "ggQuadratureDecoder.h"
#include "ggSampler.h"

#define GG_DRY_RUN

// **********************************************************************************
// Motion control points

// perimeter: 150 mm
// number of holes: 60
// drill diameter: 5 mm
// gap between holes: 2.85 mm

int mRotations = 1;
int mNumberOfHoles = 60; // # count
int mFirstHole = 1;
int mLastHole = mNumberOfHoles;
int mHoleIndexBegin = mFirstHole - 1;
int mHoleIndexEnd = mLastHole;

float mAnglePositionHome = 0.0f; // degrees
float mAnglePositionDelta = 360.0f / mNumberOfHoles; // degrees
float mAngleSpeedInit = 30.0f; // degrees per second
float mAngleSpeed = 180.0f; // degrees per second
float mAngleAcceleration = 720.0f; // degrees per second^2

float mRadiusPositionHome = 40.0f; // mm
float mRadiusPositionDrillStart = 56.0f; // mm
float mRadiusPositionDrillDelta = 0.3f; // mm
float mRadiusPositionDrillStop = mRadiusPositionDrillStart + mRadiusPositionDrillDelta; // mm
float mRadiusPositionRotationDelta = 0.5f; // mm
float mRadiusSpeedInit = 5.0f; // mm per second
float mRadiusSpeedDrilling = 0.1f; // 1/3 * 0.6 mm per second for stainless steel
float mRadiusSpeed = 13.0f; // mm per second
float mRadiusAcceleration = 100.0f; // mm per second^2

int mDrillSpeedOff = 0;
int mDrillSpeedLow = 15;
int mDrillSpeedHigh = 20;

void UpdateSettings()
{
  mHoleIndexBegin = mFirstHole - 1;
  mHoleIndexEnd = mLastHole;
  mAnglePositionDelta = 360.0f / mNumberOfHoles;
  mRadiusPositionDrillStop = mRadiusPositionDrillStart + mRadiusPositionDrillDelta;
}

boolean mRun = false;
int mHoleIndex = mHoleIndexBegin;
int mRotationIndex = 0;

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

#include "ggVector.h"
#include "ggMenu.h"
#include "ggMenuCNC.h"

#ifdef GG_DRY_RUN
ggLCD mLCD(0x26);
#else
ggLCD mLCD(0x27);
#endif

ggButtonSimple mButtonLeft(A0, true, true);
ggButtonSimple mButtonRight(A2, true, true);
ggButtonSimple mButtonUp(4/*A1*/, true, true);
ggButtonSimple mButtonDown(7/*A3*/, true, true);

ggQuadratureDecoder mCounter(0x61);

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
#ifndef GG_DRY_RUN
  mMotorAngle.Disable();
  mMotorX.Disable();
  mMotorDrill.SetSpeed(mDrillSpeedOff);
  mRun = false;
  while (!mButtonRight.SwitchingOn());
  PrintMenu(mLCD);
#endif
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

void InitMotors()
{
  mLCD.clear();
  
  mSensorAngle.begin();

  PrintMessage("Init X ...");  

  mMotorX.begin();
  mMotorX.SetUnitPerStep(mMotorXMMPerStep);
  mMotorX.SetAcceleration(mRadiusAcceleration);
  mMotorX.SetSpeed(mRadiusSpeedInit);
  mMotorX.Enable();
#ifndef GG_DRY_RUN
  mMotorX.InitPosition();
#endif

  PrintMessage("Init Angle ...");  

  mMotorAngle.begin();
  mMotorAngle.SetUnitPerStep(mMotorAngleDegreesPerStep);
  mMotorAngle.SetAcceleration(mAngleAcceleration);
  mMotorAngle.SetSpeed(mAngleSpeedInit);
  mMotorAngle.Enable();
#ifndef GG_DRY_RUN
  mMotorAngle.InitPosition();
#endif
  delay(200); // wait a wgile until system stops oscillations ...
  mSensorAngle.Set(0);
  
  PrintMessage("Power on Drill!", 0);
  PrintMessage("Click to continue.", 1);

  mMotorDrill.begin();

  while (!mButtonRight.SwitchingOn());

  mLCD.clear();

  PrintMessage("Init Drill ...");  

#ifndef GG_DRY_RUN
  while (!mMotorDrill.IsArmed()) mMotorDrill.run();
#endif
  
  mMotorDrill.SetSpeed(0);
  
  mPosition = ePositionInit;
}

void setup()
{
  mLCD.begin(20,4);
  mLCD.clear();
  
  PrintMessage("Init ...");  

  mButtonLeft.begin();
  mButtonRight.begin();
  mButtonUp.begin();
  mButtonDown.begin();
  mCounter.begin();
  
  ggSampler vSampler(2.0f);
  while (!vSampler.IsDue()) {
    if (mButtonLeft.IsOn() && mButtonRight.IsOn()) {
      PrintMessage("Reset Parameters...");
      mMenuCNC.put(EEPROM, 0);
      delay(1000);
      break;
    }
  }
  
  PrintMessage("Load Parameters...");
  if (!mMenuCNC.get(EEPROM, 0)) {
    PrintMessage("Init Parameters...");
    mMenuCNC.put(EEPROM, 0);
  }

  mMenuCNC.SelectEnter();
  PrintMenu(mLCD);
}

// **********************************************************************************
// Main program

void ActionHome()
{
  UpdateSettings();
  InitMotors();
  mRun = false;
}

void ActionRunStart()
{
  UpdateSettings();
  InitMotors();
  mHoleIndex = mHoleIndexBegin;
  mRotationIndex = 0;
  mRun = true;
}

void ActionRunContinue()
{
  UpdateSettings();
  mLCD.clear();
  mRun = true;
}

void loop()
{ 
  // step the motors as often as possible
  if (mRun) {
    mMotorAngle.run();
    mMotorX.run();
    mMotorDrill.run();
  }
  
  // set the nex destination position, if all motors arrived at their previous destination
  if (mRun &&
      !mMotorAngle.IsMoving() &&
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
        mRun = false;
      }
    }

    // move from start-position to drill-start-position
    else if (mPosition == ePositionStart) {
      PrintStatus("Move Drill down", mHoleIndex);
      CheckAngle(0.6f);
      mMotorX.SetSpeed(mRadiusSpeed);
      mMotorX.MoveToPosition(mRadiusPositionDrillStart + mRotationIndex*mRadiusPositionRotationDelta);
      mMotorDrill.SetSpeed(mDrillSpeedLow);
      mPosition = ePositionDrillStart;
    }
    
    // move from drill-start-position to drill-stop-position
    else if (mPosition == ePositionDrillStart) {
      PrintStatus("Drilling", mHoleIndex);
      CheckAngle(0.5f);
      mMotorX.SetSpeed(mRadiusSpeedDrilling);
      mMotorX.MoveToPosition(mRadiusPositionDrillStop + mRotationIndex*mRadiusPositionRotationDelta);
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
  
  // check, if user wants to intterrupt
  if (mRun && mButtonLeft.SwitchingOn()) {
    mRun = false;
    mMotorDrill.SetSpeed(0);
    PrintMenu(mLCD);
  }

  // menu control (if not running)
  if (!mRun) {
    if (mButtonRight.SwitchingOn()) {
      mMenuCNC.SelectEnter();
      if (mMenuCNC.IsModified()) {
        mLCD.clear(); mLCD.home();
        mMenuCNC.put(EEPROM, 0);
        mMenuCNC.SetModified(false);
      }
      if (!mRun) PrintMenu(mLCD);
    }
  
    if (mButtonLeft.SwitchingOn()) {
      if (mMenuCNC.GetItemSelectedLevel() > 1) {
        mMenuCNC.SelectExit();
        PrintMenu(mLCD);
      }
    }
  
    static long vCounter = mCounter.Get();
    long vCounterNew = mCounter.Get();
    long vDelta = vCounter - vCounterNew;
  
    if ((vDelta > 3) ||
        mButtonUp.SwitchingOn()) {
      vCounter = vCounterNew;
      mMenuCNC.SelectNext();
      PrintMenu(mLCD);
    }
  
    if ((vDelta < -3) ||
        mButtonDown.SwitchingOn()) {
      vCounter = vCounterNew;
      mMenuCNC.SelectPrev();
      PrintMenu(mLCD);
    }
  }
}

