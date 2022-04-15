#include "ggKey.h"
#include "ggSampler.h"
#include <EEPROM.h>


// pin connections (pwm pins: 3, 5, 6, 9, 10, 11)
#define M_KEY_A_PIN 2
#define M_KEY_B_PIN 3
#define M_KEY_P_PIN 4
#define M_LED_A_PIN 8
#define M_LED_B_PIN 7
#define M_POWER_ON_PIN 1
#define M_BATTERY_SENSOR_PIN A0



// battery sensor calibration (voltage divider)
#define M_SENSOR_OFFSET -0.372f
#define M_SENSOR_SCALE 0.0152f
#define M_BATTERY_VOLTAGE_LOW (3 * 3.7f) // 25% charge
#define M_BATTERY_VOLTAGE_EMPTY (3 * 3.55f) // 5% charge


// main control objects
ggKey mKeyA(M_KEY_A_PIN, true, true);
ggKey mKeyB(M_KEY_B_PIN, true, true);
ggKey mKeyP(M_KEY_P_PIN, true, true);


// LED brightness and color control
namespace ggLedControl {
  unsigned int mBrightness = 64; 
  unsigned int mColor = 0;
  static unsigned int GetCheckSum(unsigned int aValueA, unsigned int aValueB) {
    return B10100101 ^ aValueA ^ aValueB;
  }
  void WriteEEProm() {
    unsigned int vCheckSum = GetCheckSum(mBrightness, mColor);
    EEPROM.put(0 * sizeof(unsigned int), vCheckSum);
    EEPROM.put(1 * sizeof(unsigned int), mBrightness);
    EEPROM.put(2 * sizeof(unsigned int), mColor);
  }
  void ReadEEProm() {
    int vCheckSum, vBrightness, vColor;
    EEPROM.get(0 * sizeof(unsigned int), vCheckSum);
    EEPROM.get(1 * sizeof(unsigned int), vBrightness);
    EEPROM.get(2 * sizeof(unsigned int), vColor);
    if (vCheckSum == GetCheckSum(vBrightness, vColor)) {
      mBrightness = vBrightness;
      mColor = vColor;
    }
  }
  SetOutput(int aValueA, int aValueB) {
    analogWrite(M_LED_A_PIN, aValueA);
    analogWrite(M_LED_B_PIN, aValueB);
  }
  UpdateOutput() {
    unsigned int vValueA = (mColor * mBrightness) >> 8;
    unsigned int vValueB = ((255 - mColor) * mBrightness) >> 8;
    SetOutput(vValueA, vValueB);
  }
  void BrightnessInc() {
    mBrightness += 1 + (mBrightness >> 3);
    if (mBrightness > 255) mBrightness = 255;
  }
  void BrightnessDec() {
    mBrightness -= 1 + (mBrightness >> 3);
    if (mBrightness > 255) mBrightness = 0;
    if (mBrightness < 4) mBrightness = 4;
  }
  void ColorInc() {
    mColor += 17;
    if (mColor > 255) mColor = 255;
  }
  void ColorDec() {
    mColor -= 17;
    if (mColor > 255) mColor = 0;
  }
}


void PowerOff()
{
  digitalWrite(M_POWER_ON_PIN, false);
  while (true) {}
}


// battery control
namespace ggBattery {
  enum tState {
    eStateEmpty,
    eStateLow,
    eStateNormal
  };
  tState mState = eStateNormal;
  inline float ValueToVoltage(int aValue) {
    return M_SENSOR_SCALE * aValue + M_SENSOR_OFFSET;
  }
  inline int VoltageToValue(float aVoltage) {
    return (aVoltage - M_SENSOR_OFFSET) / M_SENSOR_SCALE;
  }
  void UpdateState() {
    if (millis() < 1000) return;
    static unsigned long vBatteryValuesSum = 0;
    static unsigned int vBatteryValuesCount = 0;
    vBatteryValuesSum += analogRead(M_BATTERY_SENSOR_PIN);
    vBatteryValuesCount++;
    if (vBatteryValuesCount >= 256) {
      int vBatteryValue = vBatteryValuesSum >> 8;
      vBatteryValuesSum = 0;
      vBatteryValuesCount = 0;
      const float vCurrent = 0.001f * ggLedControl::mBrightness;
      const float vCableResistance = 0.4; // resistance of cable
      const float vCableVoltage = vCableResistance * vCurrent;
      const float vInternalVoltage = ggBattery::ValueToVoltage(vBatteryValue);
      const float vBatteryVoltage = vInternalVoltage + vCableVoltage;
      if (vBatteryVoltage >= M_BATTERY_VOLTAGE_LOW) {
        mState = eStateNormal;
      }
      else if (vBatteryVoltage >= M_BATTERY_VOLTAGE_EMPTY) {
        if (mState == eStateNormal) {
          mState = eStateLow;
        }
      }
      else {
        mState = eStateEmpty;
      }
    }
  }
  void DisplayState() {
    static ggSampler vSampler(10.0f); // 10.0 Hz
    if (vSampler.IsDue()) {
      static unsigned int vCount = 0;
      switch (mState) {
        case eStateNormal:
          ggLedControl::UpdateOutput();
          break;
        case eStateLow:
          if (vCount == 0) ggLedControl::SetOutput(100, 0);
          else if (vCount == 1) ggLedControl::SetOutput(0, 100);
          else ggLedControl::UpdateOutput();
          break;
        case eStateEmpty:
          PowerOff();
          break;
      }
      vCount = vCount < 100 ? vCount + 1 : 0;
    }
  }
}


void setup()
{
  mKeyA.begin();
  mKeyB.begin();
  mKeyP.begin();
  pinMode(M_BATTERY_SENSOR_PIN, INPUT);
  pinMode(M_LED_A_PIN, OUTPUT); analogWrite(M_LED_A_PIN, 0);
  pinMode(M_LED_B_PIN, OUTPUT); analogWrite(M_LED_B_PIN, 0);
  pinMode(M_POWER_ON_PIN, OUTPUT); digitalWrite(M_POWER_ON_PIN, true);
  ggLedControl::ReadEEProm();
  ggLedControl::UpdateOutput();
}


void loop()
{
  // edit mode (change brightness or color)
  static bool vChangeBrightness = true;

  // check keys & adjust LED lights
  if (mKeyA.Pressed() || mKeyB.Pressed()) {
    if (mKeyA.IsOn() && mKeyB.IsOn()) {
      if (!mKeyA.IsRepeating() && !mKeyB.IsRepeating()) {
        vChangeBrightness = !vChangeBrightness;
      }
    }
    else if (mKeyA.IsOn()) {
      vChangeBrightness ? ggLedControl::BrightnessInc() : ggLedControl::ColorInc();
    }
    else if (mKeyB.IsOn()) {
      vChangeBrightness ? ggLedControl::BrightnessDec() : ggLedControl::ColorDec();
    }
    ggLedControl::WriteEEProm();
    ggLedControl::UpdateOutput();
  }

  // check power button
  if (mKeyP.Switching()) {
    static bool mPowerOff = false;
    if (mKeyP.IsOn()) mPowerOff = true;
    if (mKeyP.IsOff() && mPowerOff) PowerOff();
  }
  
  // read the battery voltage (low voltage protection)
  ggBattery::UpdateState();
  ggBattery::DisplayState();
}
