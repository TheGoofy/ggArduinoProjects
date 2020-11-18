#include "ggKey.h"
#include "ggSampler.h"
#include <EEPROM.h>


// pin connections (pwm pins: 3, 5, 6, 9, 10, 11)
#define M_KEY_A_PIN 5
#define M_KEY_B_PIN 6
#define M_LED_A_PIN 10
#define M_LED_B_PIN 11
#define M_LED_STATUS_PIN 13
#define M_BATTERY_SENSOR_PIN A4


// battery sensor calibration (voltage divider)
#define M_SENSOR_OFFSET -0.372f
#define M_SENSOR_SCALE 0.0152f
#define M_BATTERY_VOLTAGE_LOW (3 * 3.7f) // 25% charge
#define M_BATTERY_VOLTAGE_EMPTY (3 * 3.55f) // 5% charge


// main control objects
ggKey mKeyA(M_KEY_A_PIN, true, true);
ggKey mKeyB(M_KEY_B_PIN, true, true);


// helper
template <typename T>
inline T ggClamp(const T& aValue, const T& aValueMin, const T& aValueMax) {
  return aValue < aValueMin ? aValueMin : (aValueMax < aValue ? aValueMax : aValue);
}


// LED brightness
namespace ggBrightness {
  unsigned int mPower = 64; 
  unsigned int mColor = 0;
  unsigned int GetCheckSum(unsigned int aValueA, unsigned int aValueB) {
    return B10100101 ^ aValueA ^ aValueB;
  }
  void WriteEEProm() {
    unsigned int vCheckSum = GetCheckSum(mPower, mColor);
    EEPROM.put(0 * sizeof(unsigned int), vCheckSum);
    EEPROM.put(1 * sizeof(unsigned int), mPower);
    EEPROM.put(2 * sizeof(unsigned int), mColor);
  }
  void ReadEEProm() {
    int vCheckSum, vPower, vColor;
    EEPROM.get(0 * sizeof(unsigned int), vCheckSum);
    EEPROM.get(1 * sizeof(unsigned int), vPower);
    EEPROM.get(2 * sizeof(unsigned int), vColor);
    if (vCheckSum == GetCheckSum(vPower, vColor)) {
      mPower = vPower;
      mColor = vColor;
    }
  }
  SetOutput(int aValueA, int aValueB) {
    analogWrite(M_LED_A_PIN, aValueA);
    analogWrite(M_LED_B_PIN, aValueB);
  }
  UpdateOutput() {
    unsigned int vValueA = (mColor * mPower) >> 8;
    unsigned int vValueB = ((255 - mColor) * mPower) >> 8;
    SetOutput(vValueA, vValueB);
  }
  void PowerInc() {
    mPower += 1 + (mPower >> 3);
    if (mPower > 255) mPower = 255;
  }
  void PowerDec() {
    mPower -= 1 + (mPower >> 3);
    if (mPower > 255) mPower = 0;
    if (mPower < 4) mPower = 4;
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


// battery control
namespace ggBattery {
  enum tState {
    eStateEmpty,
    eStateLow,
    eStateNormal
  };
  tState mState = eStateEmpty;
  inline float ValueToVoltage(int aValue) {
    return M_SENSOR_SCALE * aValue + M_SENSOR_OFFSET;
  }
  inline int VoltageToValue(float aVoltage) {
    return (aVoltage - M_SENSOR_OFFSET) / M_SENSOR_SCALE;
  }
  void UpdateState() {
    static unsigned long vBatteryValuesSum = 0;
    static unsigned int vBatteryValuesCount = 0;
    vBatteryValuesSum += analogRead(M_BATTERY_SENSOR_PIN);
    vBatteryValuesCount++;
    if (vBatteryValuesCount >= 256) {
      int vBatteryValue = vBatteryValuesSum >> 8;
      vBatteryValuesSum = 0;
      vBatteryValuesCount = 0;
      const float vCurrent = 0.001f * ggBrightness::mPower;
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
          ggBrightness::UpdateOutput();
          break;
        case eStateLow:
          if (vCount == 0) ggBrightness::SetOutput(100, 0);
          else if (vCount == 1) ggBrightness::SetOutput(0, 100);
          else ggBrightness::UpdateOutput();
          break;
        case eStateEmpty:
          if (vCount == 0) ggBrightness::SetOutput(50, 0);
          else if (vCount == 1) ggBrightness::SetOutput(0, 50);
          else ggBrightness::SetOutput(0, 0);
          break;
      }
      vCount = vCount < 100 ? vCount + 1 : 0;
    }
  }
}


void setup()
{
  Serial.begin(115200);
  Serial.print("\nStartup ");
  mKeyA.begin(); Serial.print(".");
  mKeyB.begin(); Serial.print(".");
  pinMode(M_LED_A_PIN, OUTPUT); Serial.print(".");
  pinMode(M_LED_B_PIN, OUTPUT); Serial.print(".");
  pinMode(M_LED_STATUS_PIN, OUTPUT); Serial.print(".");
  pinMode(M_BATTERY_SENSOR_PIN, INPUT); Serial.print(".");
  ggBrightness::ReadEEProm(); Serial.print(".");
  ggBrightness::UpdateOutput(); Serial.print(".");
  Serial.print(" done!\n");
}


void loop()
{
  // edit mode (change power or color)
  static bool vChangePower = true;

  // check keys & adjust LED lights
  if (mKeyA.Pressed() || mKeyB.Pressed()) {
    if (mKeyA.IsOn() && mKeyB.IsOn()) {
      if (!mKeyA.IsRepeating() && !mKeyB.IsRepeating()) {
        vChangePower = !vChangePower;
      }
    }
    else if (mKeyA.IsOn()) {
      vChangePower ? ggBrightness::PowerInc() : ggBrightness::ColorInc();
    }
    else if (mKeyB.IsOn()) {
      vChangePower ? ggBrightness::PowerDec() : ggBrightness::ColorDec();
    }
    ggBrightness::WriteEEProm();
    ggBrightness::UpdateOutput();
  }
  
  // read the battery voltage (low voltage protection)
  ggBattery::UpdateState();
  ggBattery::DisplayState();
}
