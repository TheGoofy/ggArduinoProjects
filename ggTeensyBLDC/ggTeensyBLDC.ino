#include "ggHalfBridge.h"
#include "ggStatusLEDs.h"
#include "ggPath.h"

// enable ("inhibit" => when set to low device goes in sleep mode)
#define M_INH_U_PIN 3
#define M_INH_V_PIN 4
#define M_INH_W_PIN 5

// pwm ("input" => defines whether high- or lowside switch is activated)
#define M_IN_U_PIN 22
#define M_IN_V_PIN 21
#define M_IN_W_PIN 20

// current (Current Sense and Diagnostics)
#define M_IS_U_PIN A10
#define M_IS_V_PIN A11
#define M_IS_W_PIN A3 // 17

// phase voltage sense
#define M_VS_U_PIN A0 // 14
#define M_VS_V_PIN A1 // 15
#define M_VS_W_PIN A2 // 16

// user interface
#define M_LED_A_PIN 11
#define M_LED_B_PIN 12
#define M_LED_ON_BOARD_PIN 13
#define M_KEY_PIN 2

// pwm settings
const int mPWMResolution = 10; // 10 bit
const int mPWMValueMax = 0.95 * (1 << mPWMResolution);
const int mPWMFrequency = 10000; // Hz

// pwm setup (callback)
void SetupPWM(int aPin) {
  Serial.printf("%s - Pin: %d / Freq: %d Hz / Res: %d bit / Max: %d\n",
    __PRETTY_FUNCTION__, aPin, mPWMFrequency, mPWMResolution, mPWMValueMax);
  analogWriteResolution(mPWMResolution);
  analogWriteFrequency(aPin, mPWMFrequency);
}

// "display"
ggStatusLEDs mStatusLEDs(M_LED_A_PIN, M_LED_B_PIN, M_LED_ON_BOARD_PIN);

// half bridges (pins: current sense / enable / pwm)
ggHalfBridge mDriveU(M_IS_U_PIN, M_INH_U_PIN, M_IN_U_PIN, SetupPWM, mPWMValueMax);
ggHalfBridge mDriveV(M_IS_V_PIN, M_INH_V_PIN, M_IN_V_PIN, SetupPWM, mPWMValueMax);
ggHalfBridge mDriveW(M_IS_W_PIN, M_INH_W_PIN, M_IN_W_PIN, SetupPWM, mPWMValueMax);


ggPath mPath;

void DrivePath()
{
  delay(100);
  if (mPath.GetT() < 2.0) mPath.MoveToV(10.0, 2.0);
  else if (mPath.GetT() < 5.0) mPath.MoveToV(-5.0, 5.0);
  else mPath.MoveToV(0.0, 1.0);
  mPath.Update();
  Serial.printf("t: %f s: %f v: %f a: %f\n", mPath.GetT(), mPath.GetS(), mPath.GetV(), mPath.GetA());
}


void setup()
{
  Serial.begin(38400);
  mStatusLEDs.Begin();
  mDriveU.Begin();
  mDriveV.Begin();
  mDriveW.Begin();
  mDriveU.SetEnable(true);
  mDriveV.SetEnable(true);
  mDriveW.SetEnable(true);
}


void DriveUVW(int aZero, int aU, int aV, int aW)
{
  mDriveU.SetPWM(aZero + aU);
  mDriveV.SetPWM(aZero + aV);
  mDriveW.SetPWM(aZero + aW);
}


void loop()
{
  DrivePath();  
  /*  
  static int vCount = 0;
  mStatusLEDs.SetOnBoard(vCount > 5);
  if (++vCount > 10) vCount = 0;
  
  static const int mPWMZero = mPWMValueMax / 2;
  static const int mPWMAmpl = mPWMValueMax / 5;
  static const int vDelay = 5;

  DriveUVW(mPWMZero,         0, -mPWMAmpl,  mPWMAmpl); delay(vDelay);
  DriveUVW(mPWMZero,  mPWMAmpl, -mPWMAmpl,         0); delay(vDelay);
  DriveUVW(mPWMZero,  mPWMAmpl,         0, -mPWMAmpl); delay(vDelay);
  DriveUVW(mPWMZero,         0,  mPWMAmpl, -mPWMAmpl); delay(vDelay);
  DriveUVW(mPWMZero, -mPWMAmpl,  mPWMAmpl,         0); delay(vDelay);
  DriveUVW(mPWMZero, -mPWMAmpl,         0,  mPWMAmpl); delay(vDelay);
  */
}
