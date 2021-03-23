#include "ggHalfBridge.h"
#include "ggDualHalfBridge.h"
#include "ggFullBridge.h"


#define M_TEENSY
//#define M_ARDUINO


#ifdef M_TEENSY

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

const int mPWMResolution = 10;
const int mPWMValueMax = 0.95 * (1 << mPWMResolution);

void SetupPWM(int aPin) {
  analogWriteResolution(mPWMResolution);
  analogWriteFrequency(aPin, 20000);
  Serial.printf("mPWMResolution = %d\n", mPWMResolution);
  Serial.printf("mPWMValueMax = %d\n", mPWMValueMax);
}

ggDualHalfBridge mMotorA(M_IS_U_PIN, M_INH_U_PIN, M_IN_U_PIN,
                         M_IS_V_PIN, M_INH_V_PIN, M_IN_V_PIN,
                         &SetupPWM, mPWMValueMax);
// ggFullBridge mMotorA(5, 7, 9, &SetupPWM, mPWMValueMax);
// ggFullBridge mMotorB(6, 8, 10, &SetupPWM, mPWMValueMax);

#endif // M_TEENSY

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
  pinMode(M_LED_A_PIN, OUTPUT);
  pinMode(M_LED_B_PIN, OUTPUT);
  pinMode(M_LED_ON_BOARD_PIN, OUTPUT);
  digitalWrite(M_LED_A_PIN, LOW);
  digitalWrite(M_LED_B_PIN, LOW);
  digitalWrite(M_LED_ON_BOARD_PIN, LOW);
}


void loop()
{
  digitalWrite(M_LED_A_PIN, HIGH);
  digitalWrite(M_LED_B_PIN, LOW);
  mMotorA.SetDirection(true);

  mMotorA.SweepUp(5000);
  digitalWrite(M_LED_ON_BOARD_PIN, HIGH);
  delay(1000);
  digitalWrite(M_LED_ON_BOARD_PIN, LOW);
  mMotorA.SweepDown(500);

  digitalWrite(M_LED_A_PIN, LOW);
  digitalWrite(M_LED_B_PIN, HIGH);
  mMotorA.SetDirection(false);

  mMotorA.SweepUp(500);
  digitalWrite(M_LED_ON_BOARD_PIN, HIGH);
  delay(1000);
  digitalWrite(M_LED_ON_BOARD_PIN, LOW);
  mMotorA.SweepDown(5000);
}
