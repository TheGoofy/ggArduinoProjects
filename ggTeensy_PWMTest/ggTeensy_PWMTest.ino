#include "ggHalfBridge.h"
#include "ggDualHalfBridge.h"
#include "ggFullBridge.h"


#define M_TEENSY
//#define M_ARDUINO


#ifdef M_TEENSY
const int mPWMResolution = 10;
const int mPWMValueMax = 0.95 * (1 << mPWMResolution);
void SetupPWM(int aPin) {
  analogWriteResolution(mPWMResolution);
  analogWriteFrequency(aPin, 20000);
  Serial.printf("mPWMResolution = %d\n", mPWMResolution);
  Serial.printf("mPWMValueMax = %d\n", mPWMValueMax);
}
ggDualHalfBridge mMotorA(A4, 20, 22, A5, 21, 23, &SetupPWM, mPWMValueMax);
// ggFullBridge mMotorA(5, 7, 9, &SetupPWM, mPWMValueMax);
// ggFullBridge mMotorB(6, 8, 10, &SetupPWM, mPWMValueMax);
#endif

/*
#ifdef M_ARDUINO
const int mPWMResolution = 8;
const int mPWMValueMax = 0.95 * (1 << mPWMResolution);
void SetupPWM(int aPin) {
  // if ((aPin == 5) || (aPin == 6)) TCCR0B = TCCR0B & B11111000 | B00000010; // timer prescaler for pins 5 and 6 (this timer affects basic timing functions like "delay" or "millis")
  if ((aPin == 9) || (aPin == 10)) TCCR1B = TCCR1B & B11111000 | B00000001; // timer prescaler for pins 9 and 10
  Serial.print("mPWMResolution = "); Serial.println(mPWMResolution);
  Serial.print("mPWMValueMax = "); Serial.println(mPWMValueMax);
}
ggFullBridge mMotorA(9, 8, 7, SetupPWM, mPWMValueMax);
ggFullBridge mMotorB(10, 11, 12, SetupPWM, mPWMValueMax);
#endif
*/

void setup()
{
  Serial.begin(38400);
  mMotorA.Begin();
  // mMotorB.Begin();
  mMotorA.SetBrake(true);
  // mMotorB.SetBrake(true);
  pinMode(13, OUTPUT);
}


void loop()
{
  mMotorA.SetDirection(true);
  mMotorA.SweepUp(5000);
  delay(1000);
  mMotorA.SweepDown(500);
  mMotorA.SetDirection(false);
  mMotorA.SweepUp(500);
  delay(1000);
  mMotorA.SweepDown(5000);
}
