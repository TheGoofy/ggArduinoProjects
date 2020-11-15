#include "ggKey.h"
#include "ggSampler.h"

// pwm pins: 3, 5, 6, 9, 10, 11

#define M_KEY_A_PIN 5
#define M_KEY_B_PIN 6
#define M_LED_WHITE_PIN 10
#define M_LED_ORANGE_PIN 11
#define M_LED_STATUS_PIN 13
#define M_BATTERY_SENSOR_PIN A4


ggKey mKeyA(M_KEY_A_PIN, true, true);
ggKey mKeyB(M_KEY_B_PIN, true, true);
ggSampler mSampler(2.0f); // 2 Hz


void setup()
{
  Serial.begin(115200);
  Serial.print("\nStartup ");
  mKeyA.begin(); Serial.print(".");
  mKeyB.begin(); Serial.print(".");
  pinMode(M_LED_WHITE_PIN, OUTPUT); Serial.print(".");
  pinMode(M_LED_ORANGE_PIN, OUTPUT); Serial.print(".");
  pinMode(M_LED_STATUS_PIN, OUTPUT); Serial.print(".");
  Serial.print(" done!\n");
}


void loop()
{
  if (mKeyA.Pressed()) {
    Serial.println("Key A pressed");
  }

  if (mKeyB.Pressed()) {
    Serial.println("Key B pressed");
  }
  
  if (mSampler.IsDue()) {

    // read the battery voltage (low voltage protection)
    int vBatteryValue = analogRead(M_BATTERY_SENSOR_PIN);
    Serial.print("Battery Value = ");
    Serial.println(vBatteryValue);
    
    // initial values (255 = 3 * 5 * 17)
    static int vValue = 0;
    static int vInc = 17;

    // set outputs
    analogWrite(M_LED_WHITE_PIN, vValue);
    analogWrite(M_LED_ORANGE_PIN, 255 - vValue);
    digitalWrite(M_LED_STATUS_PIN, !digitalRead(M_LED_STATUS_PIN));

    // next iteration
    vValue += vInc;
    if (vValue > 255) { vValue = 255; vInc = -vInc; }
    if (vValue < 0) { vValue = 0; vInc = -vInc; }
  }
}
