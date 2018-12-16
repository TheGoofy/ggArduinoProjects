#include <Wire.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include "ggLCD.h"
#include "ggQuadratureDecoder.h"
#include "ggSampler.h"
#include "ggStepper.h"
#include "ggMotorController.h"
#include "ggButton2.h"


#define GG_STEPPER_ENABLE_PIN 8
#define GG_STEPPER_0_STEP_PIN 2
#define GG_STEPPER_1_STEP_PIN 3
#define GG_STEPPER_0_DIR_PIN 5
#define GG_STEPPER_1_DIR_PIN 6
#define GG_STEPPER_0_SENSOR_PIN 9
#define GG_STEPPER_1_SENSOR_PIN 10

ggStepper mMotor0(GG_STEPPER_ENABLE_PIN,
                  GG_STEPPER_0_DIR_PIN,
                  GG_STEPPER_0_STEP_PIN,
                  GG_STEPPER_0_SENSOR_PIN);
                  
ggStepper mMotor1(GG_STEPPER_ENABLE_PIN,
                  GG_STEPPER_1_DIR_PIN,
                  GG_STEPPER_1_STEP_PIN,
                  GG_STEPPER_1_SENSOR_PIN);

#define GG_MOTOR_DRILL_POWER_PIN 12
#define GG_MOTOR_DRILL_BEC_PIN A1

ggMotorController mMotorDrill(GG_MOTOR_DRILL_POWER_PIN,
                              GG_MOTOR_DRILL_BEC_PIN);

ggQuadratureDecoder mCounter0(0x60);
ggQuadratureDecoder mCounter1(0x61);

ggLCD mLCD(0x27);

ggButton2 mButton(A0, true, true);

ggSampler mSampler(10);


void PrintMessage(const char* aMessage)
{
  mLCD.setCursor(0, 0);
  mLCD.print(aMessage);
  mLCD.clearLineEnd();
}


void PrintValue(const char* aName, float aValue, byte aDecimals = 3)
{
  mLCD.setCursor(0, 0);
  mLCD.print(aName);
  mLCD.print(": ");
  mLCD.print(aValue, aDecimals);
  mLCD.clearLineEnd();
}


const long mCounterStepsPerRevolution = 4L * 400L * 128L / 16L;
const float mCounterDegreesPerStep = 360.0f / mCounterStepsPerRevolution;

const long mMotor0StepsPerRevolution = 4L * 200L * 128L / 16L;
const float mMotor0DegreesPerStep = 360.0f / mMotor0StepsPerRevolution;

const long mMotor1StepsPerRevolution = 4L * 200L;
const float mMotor1MMPerStep = 1.5f / mMotor1StepsPerRevolution;

boolean mRun = true;


void setup()
{
  mLCD.begin();
  mLCD.clear();
  
  PrintMessage("Init ...");  

  mButton.begin();

  PrintMessage("Click to init!");  
  
  while (!mButton.SwitchingOn());
  
  mCounter0.begin();

  PrintMessage("Init Motor 0 ...");  

  mMotor0.begin();
  mMotor0.SetUnitPerStep(mMotor0DegreesPerStep);
  mMotor0.Enable();
  mMotor0.SetSpeed(45.0f);
  mMotor0.InitPosition();
  mCounter0.Set(0);
  
  PrintMessage("Init Motor 1 ...");  

  mMotor1.begin();
  mMotor1.SetUnitPerStep(mMotor1MMPerStep);
  mMotor1.Enable();
  mMotor1.SetSpeed(2.0f);
  mMotor1.InitPosition();

  PrintMessage("Init Motor Drill ...");  

  mMotorDrill.begin();
  while (!mMotorDrill.IsArmed()) mMotorDrill.run();
  mMotorDrill.SetSpeed(0);

  PrintMessage("Click to run!");  
  
  while (!mButton.SwitchingOn());

  PrintMessage(mRun ? "Running" : "Pause");  
}


void loop()
{ 
  if (mSampler.IsDue()) {

    if (!mMotor0.IsMoving()) {
      if (mMotor0.GetPosition() < 1.0f) {
        mMotor0.SetSpeed(120.0f);
        mMotor0.MoveToPosition(360.0f);
        mMotorDrill.SetSpeed(0);
      }
      else {
        mMotor0.SetSpeed(60.0f);
        mMotor0.MoveToPosition(0.0f);
        mMotorDrill.SetSpeed(100);
      }
    }

    if (!mMotor1.IsMoving()) {
      if (mMotor1.GetPosition() < 1.0f) {
        mMotor1.SetSpeed(5.0f);
        mMotor1.MoveToPosition(10.0f);
      }
      else {
        mMotor1.SetSpeed(6.0f);
        mMotor1.MoveToPosition(0.0f);
      }
    }
  }
  
  if (mButton.SwitchingOn()) {
    mRun = !mRun;
    PrintMessage(mRun ? "Running" : "Pause");  
  }
    
  if (mRun) {
    mMotor0.run();
    mMotor1.run();
    mMotorDrill.run();
  }

}

