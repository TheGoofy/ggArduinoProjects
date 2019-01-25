#include "ggButton.h"
#include "ggSampler.h"

#define GG_VOLTAGE_PIN A0

#define LED_WHITE_L_PIN 3
#define LED_WHITE_R_PIN 5
#define LED_RED_L_PIN 2
#define LED_RED_R_PIN 4

#define HEAD_LIGHT_LEFT_PIN 11
#define HEAD_LIGHT_CENTER_PIN 9
#define HEAD_LIGHT_RIGHT_PIN 10

#define LED_CONTROL_RED_PIN 1
#define LED_CONTROL_GREEN_PIN 0


float mVoltage = 0.0f;
boolean mLowBattery = false;


void SampleVoltage()
{
  const float vFilter = 0.95f;
  int vValue = analogRead(GG_VOLTAGE_PIN);
  mVoltage = vFilter*mVoltage + (1.0f-vFilter)*(0.0192f*vValue);
  mLowBattery = mVoltage < 3.0f*3.3f;
}


void RedL(boolean aOn)
{
  digitalWrite(LED_RED_L_PIN, aOn);
}


void WhiteL(boolean aOn)
{
  digitalWrite(LED_WHITE_L_PIN, aOn);
}


void RedR(boolean aOn)
{
  digitalWrite(LED_RED_R_PIN, aOn);
}


void WhiteR(boolean aOn)
{
  digitalWrite(LED_WHITE_R_PIN, aOn);
}


void HeadLightLeft(int aBrightness)
{
  analogWrite(HEAD_LIGHT_LEFT_PIN, aBrightness);
}


void HeadLightCenter(int aBrightness)
{
  analogWrite(HEAD_LIGHT_CENTER_PIN, aBrightness);
}


void HeadLightRight(int aBrightness)
{
  analogWrite(HEAD_LIGHT_RIGHT_PIN, aBrightness);
}


void HeadLights(int aL, int aC, int aR)
{
  HeadLightLeft(aL);
  HeadLightCenter(aC);
  HeadLightRight(aR);
}


void Lamps(boolean aWhiteL,
           boolean aWhiteR,
           boolean aRedL,
           boolean aRedR)
{
  WhiteL(aWhiteL);
  WhiteR(aWhiteR);
  RedL(aRedL);
  RedR(aRedR);
}


ggButton mButton(8, true, true);
ggButton mButtonHeadLight(7, true, true);

ggSampler mFlashSampler(8.0f*70.0f/60.0f); // Trittfrequenz
ggSampler mVoltageSampler(20.0f);
ggSampler mVoltageIndicator(2.0f);

enum tMode {
  eModeOff,
  eModeFlash,
  eModeOnLow,
  eModeOnHigh
};

tMode mMode = eModeOff;

int mStep = 0;


void ControlRG(boolean aRedOn, boolean aGreenOn)
{
  digitalWrite(LED_CONTROL_RED_PIN, !aRedOn);
  digitalWrite(LED_CONTROL_GREEN_PIN, !aGreenOn);
}


void setup()
{
  pinMode(LED_RED_R_PIN, OUTPUT);
  pinMode(LED_RED_L_PIN, OUTPUT);
  pinMode(LED_WHITE_R_PIN, OUTPUT);
  pinMode(LED_WHITE_L_PIN, OUTPUT);
  pinMode(HEAD_LIGHT_LEFT_PIN, OUTPUT);
  pinMode(HEAD_LIGHT_CENTER_PIN, OUTPUT);
  pinMode(HEAD_LIGHT_RIGHT_PIN, OUTPUT);
  pinMode(LED_CONTROL_RED_PIN, OUTPUT);
  pinMode(LED_CONTROL_GREEN_PIN, OUTPUT);
  ControlRG(false, false);
  HeadLights(0, 0, 0);
  mButton.begin();
  mButtonHeadLight.begin();
}


boolean mHupe = false;
ggSampler mHupenSampler(8.0f);

void loop()
{
  if (mButton.SwitchingOn()) {
    switch (mMode) {
      case eModeOff: mMode = eModeFlash; break;
      case eModeFlash: mMode = mLowBattery ? eModeOff : eModeOnLow; break;
      case eModeOnLow: mMode = mLowBattery ? eModeOff : eModeOnHigh; break;
      case eModeOnHigh: mMode = eModeOff; break;
      default: break;
    }
    if (mMode == eModeOff) {
      Lamps(false, false, false, false);
      HeadLights(0, 0, 0);
    }
    if (mMode == eModeFlash) HeadLights(0, 0, 0);
    if (mMode == eModeOnLow) HeadLights(0, 50, 0);
    if (mMode == eModeOnHigh) HeadLights(255, 255, 255);
  }
  
  if (mButtonHeadLight.SwitchingOn()) {
    mHupe = true;
  }
  if (mHupe) {
    if (mHupenSampler.IsDue()) {
      mHupe = mButtonHeadLight.IsOn();
      static boolean vOn = false;
      vOn = !vOn;
      if (mHupe && vOn) {
        HeadLights(255, 255, 255);
      }
      else {
        HeadLights(0, 0, 0);
      }
    }
    if (!mHupe) {
      if (mMode == eModeOnLow) HeadLights(0, 50, 0);
      if (mMode == eModeOnHigh) HeadLights(255, 255, 255);
    }
  }
  
  if (mFlashSampler.IsDue()) {
    if (mStep >= 7) mStep = 0;
    else mStep += 1;
    if (mMode == eModeFlash || mMode == eModeOnLow || mMode == eModeOnHigh) {
      boolean vOn = (mMode == eModeOnLow) || (mMode == eModeOnHigh);
      switch (mStep) {
        case 0: Lamps(vOn || true,  vOn || false, false, false); break;
        case 1: Lamps(vOn || true,  vOn || true,  false, false); break;
        case 2: Lamps(vOn || false, vOn || true,  false, false); break;
        case 3: Lamps(vOn || false, vOn || true,  false, true ); break;
        case 4: Lamps(vOn || false, vOn || false, false, true ); break;
        case 5: Lamps(vOn || false, vOn || false, true,  true ); break;
        case 6: Lamps(vOn || false, vOn || false, true,  false); break;
        case 7: Lamps(vOn || true,  vOn || false, true,  false); break;
        default: break;
      }
    }
  }
  
  if (mVoltageSampler.IsDue()) {
    SampleVoltage();
    if (mLowBattery && (mMode == eModeOnLow || mMode == eModeOnHigh)) {
      mMode = eModeFlash;
      HeadLights(0, 0, 0);
    }
  }
  
  if (mVoltageIndicator.IsDue()) {
    static boolean vBlink = false;
    vBlink = !vBlink;
    if      (mVoltage >= 11.1f) ControlRG(false,  true);
    else if (mVoltage >= 10.5f) ControlRG(vBlink, true);
    else if (mVoltage >=  9.9f) ControlRG(true,   true);
    else if (mVoltage >=  9.4f) ControlRG(true,   vBlink);
    else if (mVoltage >=  9.0f) ControlRG(true,   false);
    else                        ControlRG(vBlink, false);
  }
}

