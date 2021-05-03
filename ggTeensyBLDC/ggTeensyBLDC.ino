#include <i2c_t3.h> // I2C wirelibrary for teensy3

#include "ggHalfBridge.h"
#include "ggStatusLEDs.h"
#include "ggButtonSimple.h"
#include "ggPath.h"
#include "ggSampler.h"
#include "ggAS5600.h"

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

// angle sensor conencted on I2C
ggAS5600 mAS5600(0x36);

// half bridges (pins: current sense / enable / pwm)
ggHalfBridge mDriveU(M_IS_U_PIN, M_INH_U_PIN, M_IN_U_PIN, SetupPWM, mPWMValueMax);
ggHalfBridge mDriveV(M_IS_V_PIN, M_INH_V_PIN, M_IN_V_PIN, SetupPWM, mPWMValueMax);
ggHalfBridge mDriveW(M_IS_W_PIN, M_INH_W_PIN, M_IN_W_PIN, SetupPWM, mPWMValueMax);


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


inline int GetSin(long aAmplitude, int aAngle)
{
  long vSin = aAmplitude * mSin[aAngle];
  vSin /= mSinAmplitude;
  if (vSin > 0) vSin += mPWMValueMin;
  if (vSin > mPWMValueMax) return mPWMValueMax;
  else return vSin;
}


void setup()
{
  SetupSin();

  Serial.begin(38400);
  while (!Serial); // wait until USB serial ready
  Serial.println("Hi Teensy BLDC!");
  
  Wire.begin(I2C_MASTER, 0, I2C_PINS_18_19, I2C_PULLUP_INT, I2C_RATE_1000);

  mStatusLEDs.Begin();
  mButton.Begin();
  mAS5600.Begin();
  mAS5600.SendAngleRequest();
  mDriveU.Begin();
  mDriveV.Begin();
  mDriveW.Begin();
  mDriveU.SetEnable(true);
  mDriveV.SetEnable(true);
  mDriveW.SetEnable(true);
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


// motor parameters
const int mNumPoles = 4;
float mPhaseAngle = 0.0f;
float mMotorAngle = 0.0f;
unsigned long mCalculationMicros = 0;


// advance angle each 50us
ggSampler mAngleSampler(50, [] (unsigned long aMicrosDelta) {

  // start measure calculation time
  unsigned long vMicrosStart = micros();
  
  // read user input
  mButton.Update();

  // calculate phase angles
  int vAngleU = static_cast<int>(mSinSamples * mPhaseAngle / M_2PI);
  int vAngleV = vAngleU + 1 * mSinSamples / 3;
  int vAngleW = vAngleU + 2 * mSinSamples / 3;
  if (vAngleU >= mSinSamples) vAngleU -= mSinSamples;
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
  // mStatusLEDs.SetOnBoard(mButton.GetOn());
  mStatusLEDs.SetOnBoard(vPWMU != 0);
  mStatusLEDs.SetA(vPWMV != 0);
  mStatusLEDs.SetB(vPWMW != 0);

/*
  // ramp up speed
  // motor has 4 pole pairs:
  // => 180 / 4 = 45 rotations per second
  // => 45 * 60 = 2700 rpm
  static const float vAccel = 0.01f * 6.0f * M_2PI; // increase speed each second by 6 pole-pairs per second
  static const float vSpeedMax = 180.0f * M_2PI; // max is 180 pole-pairs per second
  static float vSpeed = 0; // start with 0 pole-pairs per second
  float vTimeDelta = 0.000001f * aMicrosDelta;
  vSpeed += vAccel * vTimeDelta;
  if (vSpeed > vSpeedMax) vSpeed = vSpeedMax;

  // advance motor angle
  mMotorAngle += vSpeed * vTimeDelta;
  if (mMotorAngle > M_2PI) mMotorAngle -= M_2PI;

  // advance phase-angle
  mPhaseAngle += vSpeed * vTimeDelta;
  if (mPhaseAngle > M_2PI) mPhaseAngle -= M_2PI;
*/

  // angle from sensor
  uint16_t vAngle = mAS5600.GetAngle();
  mAS5600.SendAngleRequest();
  mMotorAngle = M_2PI * vAngle / 4096.0f;

  // transform to motor phase-angle
  mPhaseAngle = fmod(mNumPoles * mMotorAngle, M_2PI);

  // stop calculation time
  mCalculationMicros = micros() - vMicrosStart;

});


ggSampler mSerialPlotSampler(10000, [] (unsigned long aMicrosDelta) {
  static bool vPlotHeader = true;
  if (vPlotHeader) {
    Serial.printf("Calc-Time Motor-Angle Phase-Angle U V W\n");
    vPlotHeader = false;
  }
  Serial.printf("%d %d %d %d %d %d\n",
    mCalculationMicros,
    ggRound<int>(100.0f * mMotorAngle / M_2PI),
    ggRound<int>(100.0f * mPhaseAngle / M_2PI),
    mDriveU.GetPWM(), mDriveV.GetPWM(), mDriveW.GetPWM());

  if (mDriveU.GetPWM() > 500) {
    while (true) {
      Serial.printf("Funny PWM! ===============\n");
      Serial.printf("PWM U: %d\n", mDriveU.GetPWM());
      Serial.printf("PWM V: %d\n", mDriveV.GetPWM());
      Serial.printf("PWM W: %d\n", mDriveW.GetPWM());
      Serial.printf("mMotorAngle: %f rad\n", mMotorAngle);
      Serial.printf("mPhaseAngle: %f rad\n", mPhaseAngle);
      Serial.printf("mCalculationMicros: %d us\n", mCalculationMicros);
      delay(1000);
    }
  }
});


void loop()
{
  // advance angle
  mAngleSampler.Run();

  // some output on serial port (for debugging)
  mSerialPlotSampler.Run();
}
