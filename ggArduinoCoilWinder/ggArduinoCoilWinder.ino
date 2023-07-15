#include "ggStepperAxis.h"

/*
#define GG_STATUS_LED_PIN 2
#define GG_STEPPER_XYZ_ENABLE_PIN 8
#define GG_STEPPER_X_DIR_PIN 5
#define GG_STEPPER_Y_DIR_PIN 6
#define GG_STEPPER_Z_DIR_PIN 7
#define GG_STEPPER_X_STEP_PIN 2
#define GG_STEPPER_Y_STEP_PIN 3
#define GG_STEPPER_Z_STEP_PIN 4
#define GG_STEPPER_X_LIMIT_PIN 9
#define GG_STEPPER_Y_LIMIT_PIN 10
#define GG_STEPPER_Z_LIMIT_PIN 11
*/
#define GG_STATUS_LED_PIN 2
#define GG_STEPPER_XYZ_ENABLE_PIN 12
#define GG_STEPPER_X_DIR_PIN 26
#define GG_STEPPER_Y_DIR_PIN 27
#define GG_STEPPER_Z_DIR_PIN 14
#define GG_STEPPER_X_STEP_PIN 32
#define GG_STEPPER_Y_STEP_PIN 33
#define GG_STEPPER_Z_STEP_PIN 25
#define GG_STEPPER_X_LIMIT_PIN 19
#define GG_STEPPER_Y_LIMIT_PIN 18
#define GG_STEPPER_Z_LIMIT_PIN 5


ggStepperAxis mStepperX(GG_STEPPER_XYZ_ENABLE_PIN, GG_STEPPER_X_DIR_PIN, GG_STEPPER_X_STEP_PIN, GG_STEPPER_X_LIMIT_PIN);
ggStepperAxis mStepperY(GG_STEPPER_XYZ_ENABLE_PIN, GG_STEPPER_Y_DIR_PIN, GG_STEPPER_Y_STEP_PIN, GG_STEPPER_Y_LIMIT_PIN);
ggStepperAxis mStepperZ(GG_STEPPER_XYZ_ENABLE_PIN, GG_STEPPER_Z_DIR_PIN, GG_STEPPER_Z_STEP_PIN, GG_STEPPER_Z_LIMIT_PIN);


// linear guide: carriage driven by spindle
// #define GG_STEPPER_X_SPINDLE_PITCH 0.00175 // m per revolution (unit: m/rev)
// #define GG_STEPPER_X_SPINDLE_REDUCTION (20.0 / 32.0) // revolution ratio: motor / spindle (unit: 1)
// #define GG_STEPPER_X_MOTOR_STEPS_PER_REV (200 * 16) // steps * microsteps per motor revolution (unit: step/rev)
// #define GG_STEPPER_X_METER_PER_STEP (GG_STEPPER_X_SPINDLE_PITCH * GG_STEPPER_X_SPINDLE_REDUCTION / GG_STEPPER_X_MOTOR_STEPS_PER_REV)

// drum with pully and reduction gear
#define GG_STEPPER_X_TRANSMISSION_RATIO (18.0 / 90.0) // teeth ratio (unit: 1)
#define GG_STEPPER_X_MOTOR_STEPS_PER_REV (200.0 * 4.0) // steps * microsteps per motor revolution (unit: step/rev)
#define GG_STEPPER_X_TURNS_PER_STEP (GG_STEPPER_X_TRANSMISSION_RATIO / GG_STEPPER_X_MOTOR_STEPS_PER_REV)

const double vAccelMaxX = 2.0;
const double vSpeedMaxX = 3.0;
const double vSpeedInitX = 0.1;
const double vPositionInitSensorX = 0.0;
const double vPositionMinX = -10.0;
const double vPositionMaxX = 10.0;

// linear guide: carriage driven by timing belt
#define GG_STEPPER_Y_TOOTH_PITCH 0.002 // m per revolution (unit: m)
#define GG_STEPPER_Y_NUM_TEETH_PER_REV 20.0 // number of teeth per motor revolution (unit: 1)
#define GG_STEPPER_Y_MOTOR_STEPS_PER_REV (200.0 * 16.0) // steps * microsteps per motor revolution (unit: step/rev)
#define GG_STEPPER_Y_METER_PER_STEP (GG_STEPPER_Y_TOOTH_PITCH * GG_STEPPER_Y_NUM_TEETH_PER_REV / GG_STEPPER_Y_MOTOR_STEPS_PER_REV)

const double vAccelMaxY = 5.0;
const double vSpeedMaxY = 0.5;
const double vSpeedInitY = 0.025;
const double vPositionInitSensorY = 0.0042;
const double vPositionMinY = 0.0;
const double vPositionMaxY = 0.1275;


void InitDebug(ggStepperAxis& aAxis, float aSensorPosition, float aSpeed)
{
  // float vPositionOld = aAxis.InitPosition(aSensorPosition, aSpeed);
  // float vPositionDelta = vPositionOld - aSensorPosition;
  // Serial.printf("Init position delta: %.3f mm\n", 1000.0 * vPositionDelta);
  aAxis.MoveToSensor(aSpeed);
  Serial.printf("%.3f\n", 1000.0 * (aAxis.GetPosition() - aSensorPosition));
}


void setup()
{
  Serial.begin(115200);
  while (!Serial) {}
  Serial.print("\nStartup Coil Winder!\n");
  Serial.flush();

  Serial.print("Init IO-pins ...\n");
  pinMode(GG_STATUS_LED_PIN, OUTPUT);
  mStepperX.Begin();
  mStepperY.Begin();

  Serial.printf("Setup Motor X (%.6f turns/step)...\n", 1.0 * GG_STEPPER_X_TURNS_PER_STEP);
  mStepperX.SetUnitPerStep(GG_STEPPER_X_TURNS_PER_STEP);
  mStepperX.SetAcceleration(vAccelMaxX);
  mStepperX.SetSpeed(vSpeedMaxX);
  mStepperX.SetPosition(0.0);
  mStepperX.Enable();

  Serial.printf("Setup Motor Y (%.3f mm/step)...\n", 1000.0 * GG_STEPPER_Y_METER_PER_STEP);
  mStepperY.SetUnitPerStep(GG_STEPPER_Y_METER_PER_STEP);
  mStepperY.SetAcceleration(vAccelMaxY);
  mStepperY.SetSpeed(vSpeedMaxY);
  mStepperY.SetPosition(0.0);
  mStepperY.Enable();

  Serial.print("Init Motor X ...\n");
  mStepperX.InitPosition(vPositionInitSensorX, vSpeedInitX);

  Serial.print("Init Motor Y ...\n");
  mStepperY.InitPosition(vPositionInitSensorY, vSpeedInitY);

  Serial.print("Init complete!\n");
}


inline float GetRand(float aMin, float aMax)
{
  return aMin + (aMax - aMin) * std::rand() / RAND_MAX;
}


void MoveRandom(ggStepperAxis& aAxis,
                float aAccelMax,
                float aSpeedMax,
                float aPositionMin,
                float aPositionMax,
                Stream* aDebugStream = nullptr,
                const char* aAxisName ="",
                const char* aAxisUnitName = "",
                float aAxisUnitScale = 1.0)
{
  float vNewAccel = GetRand(0.1 * aAccelMax, aAccelMax);
  float vNewSpeed = GetRand(0.1 * aSpeedMax, aSpeedMax);
  float vNewPosition = GetRand(aPositionMin, aPositionMax);
  aAxis.SetAcceleration(vNewAccel);
  aAxis.SetSpeed(vNewSpeed);
  aAxis.MoveToPosition(vNewPosition);
  if (aDebugStream != nullptr) {
    aDebugStream->printf("Moving %s to %.3f %s (%.3f %s/s^2 %.3f %s/s)\n", aAxisName,
                         aAxisUnitScale * vNewPosition, aAxisUnitName,
                         aAxisUnitScale * vNewAccel, aAxisUnitName,
                         aAxisUnitScale * vNewSpeed, aAxisUnitName);
  }
}


void loop()
{

  mStepperX.Run();
  if (!mStepperX.IsMoving()) {
    // float vPositionCenterX = (vPositionMinX + vPositionMaxX) / 2.0;
    // mStepperX.MoveToPosition(mStepperX.GetPosition() < vPositionCenterX ? vPositionMaxX: vPositionMinX);
    // delay(500);
    MoveRandom(mStepperX, vAccelMaxX, vSpeedMaxX, vPositionMinX, vPositionMaxX, nullptr, "X", "#", 1.0);
  }

  mStepperY.Run();
  if (!mStepperY.IsMoving()) {
    MoveRandom(mStepperY, vAccelMaxY, vSpeedMaxY, vPositionMinY, vPositionMaxY, nullptr, "Y", "mm", 1000.0);
  }

}
