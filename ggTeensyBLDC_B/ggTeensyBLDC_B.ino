#include "ggHalfBridge.h"
#include "ggStatusLEDs.h"
#include "ggButtonSimple.h"
#include "ggPath.h"
#include "ggSampler.h"

/*

MIC4102: LO/HO

      |    HI     |
      |  0  |  1  |
------+-----+-----+
    0 | 0/0 | 0/1 |
LI ---+-----+-----+
    1 | 1/0 | 0/1 |
------+-----+-----+

if HI is "0" high-side output is "0", and low-side output is like LI
if LI is "0" low-side output is "0", and high-side output is like HI
if HI is "1" low-side output is "0", and high-side output is "1"
if LI is "1" outputs are switching synchronously inverted like HI

*/

// high-side / pwm
#define M_HI_A_PIN 10
#define M_HI_B_PIN 6
#define M_HI_C_PIN 4

// low-side / ls
#define M_LI_A_PIN 9
#define M_LI_B_PIN 5
#define M_LI_C_PIN 3

// voltage sense
#define M_VS_A A0
#define M_VS_B A1
#define M_VS_C A2
#define M_VS_Z A3 // 3 phase zero
#define M_VS_X A6 // input voltage +vs

// user interface
#define M_LED_A_PIN 1
#define M_LED_B_PIN 2
#define M_LED_ON_BOARD_PIN 13
#define M_KEY_PIN 0

// "display"
ggStatusLEDs mStatusLEDs(M_LED_A_PIN, M_LED_B_PIN, M_LED_ON_BOARD_PIN);

// "user input"
ggButtonSimple mButton(M_KEY_PIN, true/*invert*/, true/*pullup*/);

// pwm settings
const int mPWMResolution = 10; // 10 bit
const int mPWMValueBegin = 0;
const int mPWMValueEnd = 1 << mPWMResolution;
const int mPWMValueMin = 0.01 * (1 << mPWMResolution);
const int mPWMValueMax = 0.95 * (1 << mPWMResolution);
const int mPWMFrequency = 32000; // Hz

// pwm setup (callback)
void SetupPWM(int aPin) {
  /*
  Serial.printf("%s - Pin: %d / Freq: %d Hz / Res: %d bit / Max: %d\n",
    __PRETTY_FUNCTION__, aPin, mPWMFrequency, mPWMResolution, mPWMValueMax);
  */
  analogWriteResolution(mPWMResolution);
  analogWriteFrequency(aPin, mPWMFrequency);
}

// half bridges (pins: voltage sense / enable / pwm)
ggHalfBridge mDriveA(M_VS_A, M_LI_A_PIN, M_HI_A_PIN, SetupPWM, mPWMValueMax);
ggHalfBridge mDriveB(M_VS_B, M_LI_B_PIN, M_HI_B_PIN, SetupPWM, mPWMValueMax);
ggHalfBridge mDriveC(M_VS_C, M_LI_C_PIN, M_HI_C_PIN, SetupPWM, mPWMValueMax);


void setup()
{
  Serial.begin(38400);
  Serial.println();
  mStatusLEDs.Begin();
  mButton.Begin();
  mDriveA.Begin();
  mDriveB.Begin();
  mDriveC.Begin();
  mDriveA.SetEnable(true);
  mDriveB.SetEnable(true);
  mDriveC.SetEnable(true);
}

/*
ggSampler mBlinkA(500000, [] (unsigned long aMicrosDelta) {
  mStatusLEDs.SetA(!mStatusLEDs.GetA());
});


ggSampler mBlinkB(400000, [] (unsigned long aMicrosDelta) {
  mStatusLEDs.SetB(!mStatusLEDs.GetB());
});
*/

ggSampler mBlinkOnBoard(1000000, [] (unsigned long aMicrosDelta) {
  mStatusLEDs.SetOnBoard(!mStatusLEDs.GetOnBoard());
});


ggSampler mSweepPWM(500000, [] (unsigned long aMicrosDelta) {
  static int vMode = 0;
  int vValuePWM = 0;
  switch (vMode++) {
    case  0: mStatusLEDs.Set(0,0,0); vValuePWM = 0; break;
    case  1: mStatusLEDs.Set(0,1,0); vValuePWM = 1; break;
    case  2: mStatusLEDs.Set(0,0,1); vValuePWM = 2; break;
    case  3: mStatusLEDs.Set(0,0,1); vValuePWM = 4; break;
    case  4: mStatusLEDs.Set(0,0,0); vValuePWM = 8; break;
    case  5: mStatusLEDs.Set(0,0,0); vValuePWM = 16; break;
    case  6: mStatusLEDs.Set(0,0,1); vValuePWM = 32; break;
    case  7: mStatusLEDs.Set(0,0,1); vValuePWM = 64; break;
    case  8: mStatusLEDs.Set(0,0,0); vValuePWM = mPWMValueEnd - 1 - 64; break;
    case  9: mStatusLEDs.Set(0,0,0); vValuePWM = mPWMValueEnd - 1 - 32; break;
    case 10: mStatusLEDs.Set(0,0,1); vValuePWM = mPWMValueEnd - 1 - 16; break;
    case 11: mStatusLEDs.Set(0,0,1); vValuePWM = mPWMValueEnd - 1 - 8; break;
    case 12: mStatusLEDs.Set(0,0,0); vValuePWM = mPWMValueEnd - 1 - 4; break;
    case 13: mStatusLEDs.Set(0,0,0); vValuePWM = mPWMValueEnd - 1 - 2; break;
    case 14: mStatusLEDs.Set(1,0,1); vValuePWM = mPWMValueEnd - 1 - 1; break;
    case 15: mStatusLEDs.Set(1,1,1); vValuePWM = mPWMValueEnd - 1 - 0; break;
    default: break;
  }
  mDriveA.SetPWM(vValuePWM);
  mDriveB.SetPWM(vValuePWM);
  mDriveC.SetPWM(vValuePWM);
  if (vMode > 15) vMode = 0;
});


void loop()
{
  mSweepPWM.Run();
  // mBlinkA.Run();
  // mBlinkB.Run();
  // mBlinkOnBoard.Run();
  // mBlinkA.SetMicrosPeriod(mButton.IsOn() ? 50000 : 400000);
}
