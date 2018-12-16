// includes
#include <IRremote.h>

// HW conficuration
#define GG_IR_RECV_PIN 2
#define GG_SERIAL_SPEED 115200

// constants values
#define GG_BUTTON_0 0x00FF4AB5
#define GG_BUTTON_1 0x00FF6897
#define GG_BUTTON_2 0x00FF9867
#define GG_BUTTON_3 0x00FFB04F
#define GG_BUTTON_4 0x00FF30CF
#define GG_BUTTON_5 0x00FF18E7
#define GG_BUTTON_6 0x00FF7A85
#define GG_BUTTON_7 0x00FF10EF
#define GG_BUTTON_8 0x00FF38C7
#define GG_BUTTON_9 0x00FF5AA5
#define GG_BUTTON_OK 0x00FF02FD
#define GG_BUTTON_LEFT 0x00FF22DD
#define GG_BUTTON_RIGHT 0x00FFC23D
#define GG_BUTTON_UP 0x00FF629D
#define GG_BUTTON_DOWN 0x00FFA857
#define GG_BUTTON_STAR 0x00FF42BD
#define GG_BUTTON_HASH 0x00FF52AD
#define GG_BUTTON_REPEAT 0xFFFFFFFF

// global (static) variables
IRrecv mInfraredReceiver(GG_IR_RECV_PIN);
decode_results mInfraredReceiverResults;

const int vBrightnessRamp[] = {0,1,2,3,4,5,6,7,9,11,14,18,22,27,34,43,53,67,84,104,131,163,204,255}; // exponential
const int vBrightnessRampSize = sizeof(vBrightnessRamp) / sizeof(int);

class ggOutputDigital {
public:
  enum tMode {
    eModeNormal,
    eModeInverted
  };
  ggOutputDigital(int aPin, tMode aMode = eModeNormal)
  : mPin(aPin),
    mMode(aMode),
    mValue(LOW) {
  }  
  void Setup() {
    pinMode(mPin, OUTPUT);
  }  
  void On() {
    mValue = HIGH;
    Update();
  }
  void Off() {
    mValue = LOW;
    Update();
  }
  void Toggle() {
    mValue = mValue == HIGH ? LOW : HIGH;
    Update();
  }
  bool IsOn() const {
    return mValue == HIGH;
  }
  bool IsOff() const {
    return mValue == LOW;
  }
  void Update() {
    if (mMode == eModeNormal) {
      digitalWrite(mPin, mValue);
    }
    else {
      digitalWrite(mPin, mValue == HIGH ? LOW : HIGH);
    }
  }
protected:
  int mPin;
  tMode mMode;
  int mValue;
};


class ggOutputAnalog : public ggOutputDigital {
public:
  ggOutputAnalog(int aPin, tMode aMode = eModeNormal)
  : ggOutputDigital(aPin, aMode) {
  }
  void On() {
    mValue = vBrightnessRampSize - 1;
    Update();
  }
  void Off() {
    mValue = 0;
    Update();
  }
  void Toggle() {
    mValue = vBrightnessRampSize - mValue - 1;
    Update();
  }
  void Higher() {
    if (mValue < vBrightnessRampSize - 1) {
      mValue++;
      Update();
    }
  }
  void Lower() {
    if (mValue > 0) {
      mValue--;
      Update();
    }
  }
  void Update() {
    if (mMode == eModeNormal) {
      analogWrite(mPin, vBrightnessRamp[mValue]);
    }
    else {
      analogWrite(mPin, vBrightnessRamp[vBrightnessRampSize - mValue - 1]);
    }
  }
};


class ggLamp : public ggOutputAnalog {
public:
  ggLamp(int aPin, tMode aMode = eModeNormal)
  : ggOutputAnalog(aPin, aMode) {
  }
  void Brighter() {
    ggOutputAnalog::Higher();
  }
  void Darker() {
    ggOutputAnalog::Lower();
  }
  bool IsBright() const {
    return mValue == vBrightnessRampSize - 1;
  }
  bool IsDark() const {
    return mValue == 0;
  }
private:
  void Higher() {
  }
  void Lower() {
  }
};


enum tLightMode {
  eLightModeOff,
  eLightModeNight,
  eLightModeNightGlow,
  eLightModeNormal,
  eLightModeBright
};

tLightMode mLightMode = eLightModeNormal;
ggOutputDigital mPowerSupply(4, ggOutputDigital::eModeInverted);
ggLamp mLampNormal(8, ggLamp::eModeInverted);
ggLamp mLampWhite(7);
ggLamp mLampUV(5);
int mLightGlowMillis = 50;
boolean mLightGlowUp = false;

void LightNight()
{
  mPowerSupply.On();
  mLampNormal.Off();
  mLampWhite.Off();
  mLampUV.On();
  mLightMode = eLightModeNight;
}

void LightNightGlow()
{
  LightNight();
  mLightMode = eLightModeNightGlow;
}

void LightNormal()
{
  mPowerSupply.On();
  mLampNormal.On();
  mLampWhite.Off();
  mLampUV.Off();
  mLightMode = eLightModeNormal;
}

void LightBright()
{
  mPowerSupply.On();
  mLampNormal.On();
  mLampWhite.On();
  mLampUV.Off();
  mLightMode = eLightModeBright;
}

bool TooEarly()
{
  static unsigned long vLastMillis = 0;
  if (millis() - vLastMillis < 500) return true;
  vLastMillis = millis();
  return false;
}

void LightToggleOnOff()
{
  if (TooEarly()) return;
  static ggOutputDigital vPowerSupply = mPowerSupply;
  static ggLamp vLampNormal = mLampNormal;
  static ggLamp vLampWhite = mLampWhite;
  static ggLamp vLampUV = mLampUV;
  static tLightMode vLightMode = eLightModeNormal;
  if (mLightMode != eLightModeOff) {
    vLampNormal.Off();
    vLampWhite.Off();
    vLampUV.Off();
    vPowerSupply.Off();
    vLightMode = mLightMode;
    mLightMode = eLightModeOff;
  }
  else {
    mPowerSupply.Update();
    mLampNormal.Update();
    mLampWhite.Update();
    mLampUV.Update();
    mLightMode = vLightMode;
  }
}

void LightBrighter()
{
  if (mLightMode == eLightModeNight) {
    mLampUV.Brighter();
  }
  else if (mLightMode == eLightModeNightGlow) {
    if (mLightGlowMillis > 10) mLightGlowMillis--;
  }
}

void LightDarker()
{
  if (mLightMode == eLightModeNight) {
    mLampUV.Darker();
  }
  else if (mLightMode == eLightModeNightGlow) {
    if (mLightGlowMillis < 100) mLightGlowMillis++;
  }
}

void LightModeNext()
{
  if (TooEarly()) return;
  switch (mLightMode) {
    case eLightModeOff: LightToggleOnOff(); break;
    case eLightModeNight: LightNormal(); break;
    case eLightModeNightGlow: LightNormal(); break;
    case eLightModeNormal: LightBright(); break;
    default: break;
  }
}

void LightModePrevious()
{
  if (TooEarly()) return;
  switch (mLightMode) {
    case eLightModeOff: LightToggleOnOff(); break;
    case eLightModeNormal: LightNight(); break;
    case eLightModeBright: LightNormal(); break;
    default: break;
  }
}

void setup()
{
  Serial.begin(GG_SERIAL_SPEED);
  mInfraredReceiver.enableIRIn(); // Start the receiver
  mPowerSupply.Setup();
  mLampNormal.Setup();
  mLampWhite.Setup();
  mLampUV.Setup();
  LightNormal();
}

void loop()
{
  if (mInfraredReceiver.decode(&mInfraredReceiverResults)) {
    static unsigned long vKey = 0;
    if (mInfraredReceiverResults.value != GG_BUTTON_REPEAT) {
      vKey = mInfraredReceiverResults.value;
    }
    // Serial.println(vKey, HEX);
    switch (vKey) {
      case GG_BUTTON_1: LightNight(); break;
      case GG_BUTTON_2: LightNormal(); break;
      case GG_BUTTON_3: LightBright(); break;
      case GG_BUTTON_4: LightNightGlow(); break;
      case GG_BUTTON_OK: LightToggleOnOff(); break;
      case GG_BUTTON_UP: LightBrighter(); break;
      case GG_BUTTON_DOWN: LightDarker(); break;
      case GG_BUTTON_RIGHT: LightModeNext(); break;
      case GG_BUTTON_LEFT: LightModePrevious(); break;
      default: break;
    }    
    mInfraredReceiver.resume();
  }
  if (mLightMode == eLightModeNightGlow) {
    static unsigned long vLastMillis = 0;
    if (millis() - vLastMillis >= mLightGlowMillis) {
      vLastMillis = millis();
      if (mLightGlowUp) {
        if (!mLampUV.IsBright()) mLampUV.Brighter();
        if (mLampUV.IsBright()) mLightGlowUp = false;
      }
      else {
        if (!mLampUV.IsDark()) mLampUV.Darker();
        if (mLampUV.IsDark()) mLightGlowUp = true;
      }
    }
  }
}
