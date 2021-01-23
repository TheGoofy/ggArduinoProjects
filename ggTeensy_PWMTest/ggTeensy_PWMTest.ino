#include "ggHalfBridge.h"
#include "ggFullBridge.h"

// #define M_TEENSY
#define M_ARDUINO

#ifdef M_TEENSY
const int mPWMResolution = 10;
const int mPWMValueMax = 0.95 * (1 << mPWMResolution);
void SetupPWM(int aPin) {
  analogWriteResolution(mPWMResolution);
  analogWriteFrequency(aPin, 30000);
  Serial.printf("mPWMResolution = %d\n", mPWMResolution);
  Serial.printf("mPWMValueMax = %d\n", mPWMValueMax);
}
// ggHalfBridge mHalfBridgeL(A0, 3, 5, mPWMFrequency);
// ggHalfBridge mHalfBridgeR(A1, 4, 6, mPWMFrequency);
ggFullBridge mFullBridgeA(5, 7, 9, &SetupPWM, mPWMValueMax);
ggFullBridge mFullBridgeB(6, 8, 10, &SetupPWM, mPWMValueMax);
#endif

#ifdef M_ARDUINO
const int mPWMResolution = 8;
const int mPWMValueMax = 0.95 * (1 << mPWMResolution);
void SetupPWM(int aPin) {
  if ((aPin == 5) || (aPin == 6)) TCCR0B = TCCR0B & B11111000 | B00000010; // timer prescaler for pins 5 and 6 (this timer affects basic timing functions like "delay" or "millis")
  if ((aPin == 9) || (aPin == 10)) TCCR1B = TCCR1B & B11111000 | B00000010; // timer prescaler for pins 9 and 10
  Serial.print("mPWMResolution = "); Serial.println(mPWMResolution);
  Serial.print("mPWMValueMax = "); Serial.println(mPWMValueMax);
}
ggFullBridge mFullBridgeA(9, 8, 7, SetupPWM, mPWMValueMax);
ggFullBridge mFullBridgeB(10, 11, 12, SetupPWM, mPWMValueMax);
#endif


void setup()
{
  Serial.begin(38400);
  /*
  mHalfBridgeL.Begin();
  mHalfBridgeR.Begin();
  mHalfBridgeL.SetEnable(true);
  mHalfBridgeR.SetEnable(true);
  */
  mFullBridgeA.Begin();
  mFullBridgeB.Begin();
  mFullBridgeA.SetBrake(true);
  mFullBridgeB.SetBrake(true);
}


void loop()
{
  /*
  mHalfBridgeL.SweepUp(5000);
  delay(1000);
  mHalfBridgeL.SweepDown(500);
  mHalfBridgeR.SweepUp(500);
  delay(1000);
  mHalfBridgeR.SweepDown(5000);
  */
  mFullBridgeA.SetDirection(true);
  mFullBridgeA.SweepUp(5000);
  delay(1000);
  mFullBridgeA.SweepDown(500);
  mFullBridgeA.SetDirection(false);
  mFullBridgeA.SweepUp(500);
  delay(1000);
  mFullBridgeA.SweepDown(5000);
  /*
  mFullBridgeB.SetDirection(true);
  mFullBridgeB.SweepUp(5000);
  delay(1000);
  mFullBridgeB.SweepDown(500);
  mFullBridgeB.SetDirection(false);
  mFullBridgeB.SweepUp(500);
  delay(1000);
  mFullBridgeB.SweepDown(5000);
  */
}
