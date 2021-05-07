#include <i2c_t3.h> // I2C wirelibrary for teensy3

#include "ggHalfBridge.h"
#include "ggStatusLEDs.h"
#include "ggButtonSimple.h"
#include "ggPath.h"
#include "ggSampler.h"
#include "ggAS5600.h"
#include "ggHistogramT.h"

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

// external sensor (connector)
#define M_DATA1_PIN 10
#define M_DATA2_PIN 9
#define M_DATA3_PIN 8
#define M_DATA4_PIN 7
#define M_DATA5_SCL_PIN 19 // A5
#define M_DATA6_SDA_PIN 18 // A4

// xternal sensor AS5600
#define M_AS5600_OUT_PIN M_DATA2_PIN // output PWM
#define M_AS5600_PGO_PIN M_DATA1_PIN // input GND => programming mode (internal pull-up)
#define M_AS5600_SDA_PIN M_DATA6_SDA_PIN // I2C: consider external pull-up
#define M_AS5600_SCL_PIN M_DATA5_SCL_PIN // I2C: consider external pull-up
#define M_AS5600_DIR_PIN M_DATA4_PIN // input direction angle increment: GND CW / VDD CCW

// user interface
#define M_LED_A_PIN 11
#define M_LED_B_PIN 12
#define M_LED_ON_BOARD_PIN 13
#define M_KEY_PIN 2

// pwm settings
const int mPWMResolution = 10; // 10 bit
const int mPWMValueBegin = 0;
const int mPWMValueEnd = 1 << mPWMResolution;
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
ggAS5600 mAS5600(0x36, M_AS5600_DIR_PIN);

// half bridges (pins: current sense / enable / pwm)
ggHalfBridge mDriveU(M_IS_U_PIN, M_INH_U_PIN, M_IN_U_PIN, SetupPWM, mPWMValueMax);
ggHalfBridge mDriveV(M_IS_V_PIN, M_INH_V_PIN, M_IN_V_PIN, SetupPWM, mPWMValueMax);
ggHalfBridge mDriveW(M_IS_W_PIN, M_INH_W_PIN, M_IN_W_PIN, SetupPWM, mPWMValueMax);


void FatalEnd(const char* aInfo = nullptr);

#define GG_STR(aArg) GG_STR1(aArg)
#define GG_STR1(aArg) #aArg

#define GG_ASSERT(aAssertion) \
if (!(aAssertion)) { \
  FatalEnd("" __FILE__ "(" GG_STR(__LINE__) "): Assertion failed: '" #aAssertion "'");\
}

template <typename TOut>
TOut ggRound(float aValue)
{
  return static_cast<TOut>(aValue >= 0.0f ? aValue + 0.5f : aValue - 0.5f);
}

#define M_2PI 6.2831853f // 2 * Pi
#define M_PI2 1.5707963f  // 1/2 * Pi
#define M_2PI3 2.0943951f // 2/3 * Pi


// sinus lookup table
const int mSinSamples = 4096 / 4;
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
  GG_ASSERT((0 <= aAngle) && (aAngle < mSinSamples));
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
  // Serial.println("Hi Teensy BLDC!");
  
  Wire.begin(I2C_MASTER, 0, I2C_PINS_18_19, I2C_PULLUP_INT, 1000000);

  mStatusLEDs.Begin();
  mButton.Begin();

  mDriveU.Begin();
  mDriveV.Begin();
  mDriveW.Begin();
  mDriveU.SetEnable(true);
  mDriveV.SetEnable(true);
  mDriveW.SetEnable(true);
  
  mAS5600.Begin();
  mAS5600.SetDirCCW();
  mAS5600.WriteCONFSlowFilter(ggAS5600::eSlowFilter02x);
  mAS5600.WriteCONFFastFilterThreshold(ggAS5600::eFastFilterThresholdSFOnly);

  Serial.println("AS5600 Registers:");
  mAS5600.Print(Serial);

  bool vSuccess = mAS5600.SendAngleRequest();
  if (!vSuccess) Serial.println("Failed to send Angle Request!");
  delayMicroseconds(1000); // 1ms should be sufficient for receiving the first angle
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
uint16_t mPhaseAngle = 0;
uint16_t mMotorAngle = 0;


ggHistogramT<uint32_t, 32> mHistogramMicrosSample;
ggHistogramT<uint32_t, 64> mHistogramMicrosCalc;
ggHistogramT<uint32_t, 256> mHistogramAnglesDelta;


const unsigned long mSampleMicros = 50;

// advance angle each 50us
ggSampler mAngleSampler(mSampleMicros, [] (unsigned long aMicrosDelta) {

  // start measure calculation time
  const unsigned long vMicrosStart = micros();
  
  // read user input
  mButton.Update();

  // read angle from sensor
  bool vAngleNotReady = mAS5600.AngleRequestPending();
  int16_t vAngle = mAS5600.GetAngle();
  bool vAngleRequestSent = mAS5600.SendAngleRequest();
  int16_t vAngleDelta = mMotorAngle - vAngle;
  mMotorAngle = vAngle;

  // transform to motor phase-angle
  mPhaseAngle = mMotorAngle % mSinSamples;

  // calculate UVW phase angles
  int vAngleU = mPhaseAngle;
  int vAngleV = vAngleU + 1 * mSinSamples / 3;
  int vAngleW = vAngleU + 2 * mSinSamples / 3;
  if (vAngleU >= mSinSamples) vAngleU -= mSinSamples;
  if (vAngleV >= mSinSamples) vAngleV -= mSinSamples;
  if (vAngleW >= mSinSamples) vAngleW -= mSinSamples;

  // amplitude
  static const long vPWMAmpl = mPWMValueMax / 4;

  // phase pwm values
  int vPWMU = GetSin(vPWMAmpl, vAngleU);
  int vPWMV = GetSin(vPWMAmpl, vAngleV);
  int vPWMW = GetSin(vPWMAmpl, vAngleW);

  // set the pwm values
  DriveUVW(vPWMU, vPWMV, vPWMW);

  // display angle status
  mStatusLEDs.SetOnBoard(vAngleU < mSinSamples / 2);
  mStatusLEDs.SetA(vAngleV < mSinSamples / 2);
  mStatusLEDs.SetB(vAngleW < mSinSamples / 2);

  // stop calculation time
  const unsigned long vMicrosCalc = micros() - vMicrosStart;

  // do some checks (for debugging)
  mHistogramMicrosSample.Count(aMicrosDelta - mSampleMicros);
  mHistogramMicrosCalc.Count(vMicrosCalc);
  mHistogramAnglesDelta.Count(vAngleDelta + mHistogramAnglesDelta.GetSize() / 2);
  GG_ASSERT(!vAngleNotReady);
  GG_ASSERT(vAngleRequestSent);
});


ggSampler mSerialPlotSampler(10000, [] (unsigned long aMicrosDelta) {

  static bool vPlotHeader = true;
  if (vPlotHeader) {
    Serial.printf("Motor-Angle Phase-Angle U V W\n");
    vPlotHeader = false;
  }
  Serial.printf("%d %d %d %d %d\n",
    mMotorAngle / 8 / mNumPoles,
    mPhaseAngle / 8,
    mDriveU.GetPWM(), mDriveV.GetPWM(), mDriveW.GetPWM());

/*
  Serial.print("Micros Sample: ");
  mHistogramMicrosSample.PrintT(Serial);
  mHistogramMicrosSample.Clear();
  Serial.print("Micros Calc: ");
  mHistogramMicrosCalc.PrintT(Serial);
  mHistogramMicrosCalc.Clear();  
  Serial.print("Angles Delta: ");
  mHistogramAnglesDelta.PrintT(Serial);
  mHistogramAnglesDelta.Clear();
*/
});


void FatalEnd(const char* aInfo = nullptr)
{
  Serial.printf("Fatal End! ===============\n");
  if (aInfo != nullptr) Serial.printf("%s\n", aInfo);
  Serial.printf("mMotorAngle: %f rad\n", mMotorAngle);
  Serial.printf("mPhaseAngle: %f rad\n", mPhaseAngle);
  Serial.printf("PWM U: %d\n", mDriveU.GetPWM());
  Serial.printf("PWM V: %d\n", mDriveV.GetPWM());
  Serial.printf("PWM W: %d\n", mDriveW.GetPWM());
  Serial.flush();
  while (true);  
}


void loop()
{
  // advance angle
  mAngleSampler.Run();

  // some output on serial port (for debugging)
  mSerialPlotSampler.Run();
}
