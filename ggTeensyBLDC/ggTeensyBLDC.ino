#include "ggHalfBridge.h"
#include "ggStatusLEDs.h"
#include "ggButtonSimple.h"
#include "ggPath.h"
#include "ggSampler.h"

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
const int mPWMValueMin = 0.01 * (1 << mPWMResolution);
const int mPWMValueMax = 0.95 * (1 << mPWMResolution);
const int mPWMFrequency = 15000; // Hz

// pwm setup (callback)
void SetupPWM(int aPin) {
  /*
  Serial.printf("%s - Pin: %d / Freq: %d Hz / Res: %d bit / Max: %d\n",
    __PRETTY_FUNCTION__, aPin, mPWMFrequency, mPWMResolution, mPWMValueMax);
  */
  analogWriteResolution(mPWMResolution);
  analogWriteFrequency(aPin, mPWMFrequency);
}

// "display"
ggStatusLEDs mStatusLEDs(M_LED_A_PIN, M_LED_B_PIN, M_LED_ON_BOARD_PIN);

// "user input"
ggButtonSimple mButton(M_KEY_PIN, true/*invert*/, true/*pullup*/);

// half bridges (pins: current sense / enable / pwm)
ggHalfBridge mDriveU(M_IS_U_PIN, M_INH_U_PIN, M_IN_U_PIN, SetupPWM, mPWMValueMax);
ggHalfBridge mDriveV(M_IS_V_PIN, M_INH_V_PIN, M_IN_V_PIN, SetupPWM, mPWMValueMax);
ggHalfBridge mDriveW(M_IS_W_PIN, M_INH_W_PIN, M_IN_W_PIN, SetupPWM, mPWMValueMax);

/*
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
*/

template <typename TOut>
TOut ggRound(float aValue)
{
  return static_cast<TOut>(aValue >= 0.0f ? aValue + 0.5f : aValue - 0.5f);
}

#define M_2PI 6.2831853f // 2 * Pi
#define M_PI2 1.5707963f  // 1/2 * Pi
#define M_2PI3 2.0943951f // 2/3 * Pi


// sinus lookup table
const int mSinSamples = 3600; // 3600;
const int mSinAmplitude = 10000;
int mSin[mSinSamples];

// calculate sinus lookup table
void SetupSin()
{
  for (int vIndex = 0; vIndex < mSinSamples; vIndex++) {
    float vAlpha = M_2PI * (float)vIndex / (float)mSinSamples;
    vAlpha = M_2PI3 - abs(M_2PI3 - vAlpha);
    float vSin = mSinAmplitude * sin(vAlpha);
    mSin[vIndex] = ggRound<int>(vSin);
    if (mSin[vIndex] < 0) mSin[vIndex] = 0;
  }
}


void setup()
{
  SetupSin();
  Serial.begin(38400);
  Serial.println();
  mStatusLEDs.Begin();
  mButton.Begin();
  mDriveU.Begin();
  mDriveV.Begin();
  mDriveW.Begin();
  mDriveU.SetEnable(true);
  mDriveV.SetEnable(true);
  mDriveW.SetEnable(true);
}


inline int GetSin(int aAmplitude, int aAngle)
{
  long vSin = aAmplitude * mSin[aAngle];
  vSin /= mSinAmplitude;
  if (vSin > 0) vSin += mPWMValueMin;
  if (vSin > mPWMValueMax) return mPWMValueMax;
  else return vSin;
}


void DriveUVW(int aU, int aV, int aW)
{
  mDriveU.SetPWM(aU);
  mDriveV.SetPWM(aV);
  mDriveW.SetPWM(aW);
  if (mButton.GetOn()) {
    mDriveU.SetEnable((aU == 0) || ((aU > aV) && (aU > aW)));
    mDriveV.SetEnable((aV == 0) || ((aV > aU) && (aV > aW)));
    mDriveW.SetEnable((aW == 0) || ((aW > aU) && (aW > aV)));
  }
}


// main angle
float mAngle = 0.0;


// advance angle each 50us
ggSampler mAngleSampler(50, [] (unsigned long aMicrosDelta) {

  // read user input
  mButton.Update();

  // calculate phase angles
  int vAngleU = ggRound<int>(mSinSamples * mAngle / M_2PI);
  int vAngleV = vAngleU + 1 * mSinSamples / 3;
  int vAngleW = vAngleU + 2 * mSinSamples / 3;
  if (vAngleV >= mSinSamples) vAngleV -= mSinSamples;
  if (vAngleW >= mSinSamples) vAngleW -= mSinSamples;

  // amplitude
  static const long vPWMAmpl = 0.25*mPWMValueMax;

  // phase pwm values
  int vPWMU = GetSin(vPWMAmpl, vAngleU);
  int vPWMV = GetSin(vPWMAmpl, vAngleV);
  int vPWMW = GetSin(vPWMAmpl, vAngleW);

  // set the pwm values
  DriveUVW(vPWMU, vPWMV, vPWMW);

  // display angle status
  // mStatusLEDs.SetOnBoard(2 * mAngle / mSinSamples % 2);
  // mStatusLEDs.SetA(6 * mAngle / mSinSamples % 2);
  mStatusLEDs.SetOnBoard(mButton.GetOn());
  // mStatusLEDs.SetOnBoard(vPWMU != 0);
  mStatusLEDs.SetA(vPWMV != 0);
  mStatusLEDs.SetB(vPWMW != 0);

  // ramp up speed
  // motor has 4 pole pairs:
  // => 180 / 4 = 45 rotations per second
  // => 45 * 60 = 2700 rpm
  static const float vAccel = 6.0f * M_2PI; // increase speed each second by 6 pole-pairs per second
  static const float vSpeedMax = 180.0f * M_2PI; // max is 180 pole-pairs per second
  static float vSpeed = 0; // start with 0 pole-pairs per second
  float vTimeDelta = 0.000001f * aMicrosDelta;
  vSpeed += vAccel * vTimeDelta;
  if (vSpeed > vSpeedMax) vSpeed = vSpeedMax;

  // advance phase-angle
  mAngle += vSpeed * vTimeDelta;
  if (mAngle > M_2PI) mAngle -= M_2PI;
});


ggSampler mSerialPlotSampler(20000, [] (unsigned long aMicrosDelta) {
  static bool vPlotHeader = true;
  if (vPlotHeader) {
    Serial.printf("Angle U V W\n");
    vPlotHeader = false;
  }
  Serial.printf("%d %d %d %d\n", ggRound<int>(100.0f * mAngle / M_2PI), mDriveU.GetPWM(), mDriveV.GetPWM(), mDriveW.GetPWM());
});


void loop()
{
  // advance angle
  mAngleSampler.Run();

  // some output on serial port (for debugging)
  // mSerialPlotSampler.Run();
}
