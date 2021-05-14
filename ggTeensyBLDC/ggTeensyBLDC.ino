#include <i2c_t3.h> // I2C wirelibrary for teensy3

#include "ggHalfBridge.h"
#include "ggStatusLEDs.h"
#include "ggButtonSimple.h"
#include "ggPath.h"
#include "ggSampler.h"
#include "ggAS5600.h"
#include "ggHistogramT.h"
#include "ggLookupSinT.h"
#include "ggLookupSinusoidT.h"

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
const uint8_t mPWMResolution = 10; // 10 bit
const uint16_t mPWMValueBegin = 0;
const uint16_t mPWMValueEnd = 1 << mPWMResolution;
const uint16_t mPWMValueLow = 0.01 * (1 << mPWMResolution);
const uint16_t mPWMValueMax = 0.95 * (1 << mPWMResolution);
const uint16_t mPWMFrequency = 15000; // Hz

// pwm setup (callback)
void SetupPWM(uint8_t aPin) {
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

// motor parameters
const uint8_t mNumPoles = 4;
uint16_t mMotorAngle = 0;

// lookup table for sinusoidal signal: 10 bit angle resolution (1024) => 1/4 of Angle Sensor AS5600
ggLookupSinusoidT<uint16_t, 10> mSin;

void FatalEnd(const char* aInfo);

#define GG_STR(aArg) GG_STR1(aArg)
#define GG_STR1(aArg) #aArg

#define GG_ASSERT(aAssertion) \
if (!(aAssertion)) { \
  FatalEnd("" __FILE__ "(" GG_STR(__LINE__) "): Assertion failed: '" #aAssertion "'");\
}


void setup()
{
  Serial.begin(38400);
  while (!Serial && (millis() < 1000)); // wait for max. 1 second until USB serial ready
  // Serial.println("Hi Teensy BLDC!");
  // mSin.PrintT(Serial);
  
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


inline void ClampPWM(uint16_t& aValue)
{
  if (aValue > 0) aValue += mPWMValueLow;
  if (aValue > mPWMValueMax) aValue = mPWMValueMax;
}


void DriveUVW(uint16_t aU, uint16_t aV, uint16_t aW)
{
  ClampPWM(aU);
  ClampPWM(aV);
  ClampPWM(aW);
  mDriveU.SetPWM(aU);
  mDriveV.SetPWM(aV);
  mDriveW.SetPWM(aW);
  /*
  bool vEnableUVW = !mButton.GetOn();
  // disable phase with mid-value (maybe useful for measuring back-emf)
  mDriveU.SetEnable(vEnableUVW || (aU == 0) || ((aU > aV) && (aU > aW)));
  mDriveV.SetEnable(vEnableUVW || (aV == 0) || ((aV > aU) && (aV > aW)));
  mDriveW.SetEnable(vEnableUVW || (aW == 0) || ((aW > aU) && (aW > aV)));
  */
}


// for debugging
ggHistogramT<uint32_t, 32> mHistogramMicrosSample;
ggHistogramT<uint32_t, 64> mHistogramMicrosCalc;
ggHistogramT<uint32_t, 256> mHistogramAnglesDelta;

/*
4 poles, min 10 samples per pole => min 40 samples per rotation
AS5600 update rate is 6.6kHz
=> max 165 rotations per sec (9'900 rpm)
TLE5012 update rate is 23.4kHz
=> max 585 rotations per sec (35'100 rpm)
*/

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

  // transform motor-angle to phase-angles
  // motor-angle: 0..4095 (12-bit sensor resolution)
  // sin-lookup: 0..1023 (10-bit resolution - 0x03FF)
  // => suits well with 4 magnet pole pairs
  // for powers of 2, the bit-mask is a very fast modulo!
  // alternatively: vAngleU = mMotorAngle % mSin.GetPeriod();

  uint16_t vAngleU = mMotorAngle & mSin.GetPeriodMask();
  uint16_t vAngleV = (vAngleU + 1 * mSin.GetPeriod() / 3) & mSin.GetPeriodMask();
  uint16_t vAngleW = (vAngleU + 2 * mSin.GetPeriod() / 3) & mSin.GetPeriodMask();

  // amplitude
  static const uint16_t vPWMAmpl = mPWMValueMax / 4;

  // phase pwm values
  uint16_t vPWMU = mSin.Get(vPWMAmpl, vAngleU);
  uint16_t vPWMV = mSin.Get(vPWMAmpl, vAngleV);
  uint16_t vPWMW = mSin.Get(vPWMAmpl, vAngleW);

  // set the pwm values
  DriveUVW(vPWMU, vPWMV, vPWMW);

  // display angle status
  mStatusLEDs.SetOnBoard(vAngleU < mSin.GetPeriod() / 2);
  mStatusLEDs.SetA(vAngleV < mSin.GetPeriod() / 2);
  mStatusLEDs.SetB(vAngleW < mSin.GetPeriod() / 2);

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
    Serial.printf("Motor-Angle U V W\n");
    vPlotHeader = false;
  }
  Serial.printf("%d %d %d %d\n",
    mMotorAngle / 8 / mNumPoles,
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


void FatalEnd(const char* aInfo)
{
  mDriveU.SetEnable(false);
  mDriveV.SetEnable(false);
  mDriveW.SetEnable(false);
  mStatusLEDs.Set(true, true, true);
  Serial.printf("Fatal End! ===============\n");
  if (aInfo != nullptr) Serial.printf("%s\n", aInfo);
  Serial.printf("mMotorAngle: %f rad\n", mMotorAngle);
  Serial.printf("PWM U: %d\n", mDriveU.GetPWM());
  Serial.printf("PWM V: %d\n", mDriveV.GetPWM());
  Serial.printf("PWM W: %d\n", mDriveW.GetPWM());
  Serial.flush();
  while (true) {
    for (int vCount = 0; vCount < 3; vCount++) {
      mStatusLEDs.Set(true, true, true); delay(100);
      mStatusLEDs.Set(false, false, false); delay(100);
    }
    delay(1000-3*(100+100));
  }
}


void loop()
{
  // advance angle
  mAngleSampler.Run();

  // some output on serial port (for debugging)
  mSerialPlotSampler.Run();
}
