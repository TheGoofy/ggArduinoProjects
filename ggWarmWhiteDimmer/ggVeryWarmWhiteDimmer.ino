#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "ggLCD.h"
#include "ggJoyStick.h"

// red, green, blue, yellow, white
//const int vLamps = 5;
#define vLamps 5
#define vKeyPoints 6

const int vLampsPin[vLamps] = {
  9, 10, 11, 5, 6
};

const int vLampsOff[vLamps] = {
  0, 0, 0, 45, 55
};

const int vLampsOn[vLamps] = {
  255, 255, 255, 235, 245
};

/*
// black, red, green, blue, yellow, whitr
const int vKeyPoints = 6;
const float vBrightness[vKeyPoints][vLamps] = {
  {0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
  {1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
  {0.0f, 1.0f, 0.0f, 0.0f, 0.0f},
  {0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
  {0.0f, 0.0f, 0.0f, 1.0f, 0.0f},
  {0.0f, 0.0f, 0.0f, 0.0f, 1.0f}
};
*/

// ambiance dimmer: black, bordeau, red, orange, yellow, white
//const int vKeyPoints = 6;
const float vAmbianceDimmer[vKeyPoints][vLamps] = {
  {0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
  {0.3f, 0.05f, 0.0f, 0.0f, 0.0f},
  {0.9f, 0.2f, 0.0f, 0.1f, 0.0f},
  {1.0f, 0.4f, 0.0f, 0.5f, 0.1f},
  {1.0f, 0.8f, 0.1f, 0.9f, 0.5f},
  {1.0f, 1.0f, 0.4f, 1.0f, 0.9f}
};

/*
// rainbow A
//const int vKeyPoints = 7;
const float vRainbowA[vKeyPoints][vLamps] = {
  {1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
  {0.7f, 0.7f, 0.0f, 0.0f, 0.0f},
  {0.0f, 1.0f, 0.0f, 0.0f, 0.0f},
  {0.0f, 0.7f, 0.7f, 0.0f, 0.0f},
  {0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
  {0.7f, 0.0f, 0.7f, 0.0f, 0.0f},
  {1.0f, 0.0f, 0.0f, 0.0f, 0.0f}
};
*/
/*
// rainbow B
const int vKeyPoints = 9;
const float vBrightness[vKeyPoints][vLamps] = {
  {0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
  {1.0f, 0.0f, 0.0f, 0.1f, 0.0f},
  {0.7f, 0.7f, 0.0f, 0.2f, 0.0f},
  {0.0f, 1.0f, 0.0f, 0.0f, 0.0f},
  {0.0f, 0.7f, 0.7f, 0.0f, 0.0f},
  {0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
  {0.7f, 0.0f, 0.7f, 0.0f, 0.0f},
  {1.0f, 0.7f, 1.0f, 0.5f, 0.7f},
  {1.0f, 1.0f, 1.0f, 1.0f, 1.0f}
};
*/

/*
void TestCalibration()
{
  const int vBlinks = 1;
  const int vBlinkDelay = 100;
  for (int vLamp = 0; vLamp < vLamps; vLamp++) {
    // test off: blink real of", and calibrated off
    for (int vBlink = 0; vBlink < vBlinks; vBlink++) {
      PWM_Write(vLampsPin[vLamp], 0); // turn lam really off
      digitalWrite(13, LOW); // control LED on board
      delay(vBlinkDelay); // wait a bit
      PWM_Write(vLampsPin[vLamp], vLampsOff[vLamp]); // calibrated "off"
      digitalWrite(13, HIGH); // control LED on board
      delay(vBlinkDelay); // wait a bit
    }
    // test on: blink real on, and calibrated on
    for (int vBlink = 0; vBlink < vBlinks; vBlink++) {
      PWM_Write(vLampsPin[vLamp], 255); // turn lam really on
      digitalWrite(13, LOW); // control LED on board
      delay(vBlinkDelay); // wait a bit
      PWM_Write(vLampsPin[vLamp], vLampsOn[vLamp]); // calibrated "on"
      digitalWrite(13, HIGH); // control LED on board
      delay(vBlinkDelay); // wait a bit
    }
    // switch it off again
    PWM_Write(vLampsPin[vLamp], 0); // turn lam really off
  }
}
*/
void SetLamp(int aLamp, float aBrightness)
{
  int vBrightness = 0.5f + vLampsOff[aLamp] + aBrightness * (vLampsOn[aLamp] - vLampsOff[aLamp]);
  analogWrite(vLampsPin[aLamp], vBrightness);
}

void SetLamps(const float aColor[vLamps])
{
  for (int vLamp = 0; vLamp < vLamps; vLamp++) {
    SetLamp(vLamp, aColor[vLamp]);
  }
}

float SetLamps(const float aColors[vKeyPoints][vLamps], float aBrightness)
{
  float vValue = aBrightness * (vKeyPoints - 1);
  int vKeyPointA = floor(vValue);
  int vKeyPointB = vKeyPointA + 1;
  float vWeightB = vValue - vKeyPointA;
  float vWeightA = 1.0f - vWeightB;
  for (int vLamp = 0; vLamp < vLamps; vLamp++) {
    SetLamp(vLamp, vWeightA * aColors[vKeyPointA][vLamp] + vWeightB * aColors[vKeyPointB][vLamp]);
  }
}
/*
void TestKeyPoints(const float aColors[vKeyPoints][vLamps], int aDelay = 500)
{
  for (int vKeyPoint = 0; vKeyPoint < vKeyPoints; vKeyPoint++) {
    SetLamps(aColors[vKeyPoint]);
    delay(aDelay);
  }
}
*/
void TestInterpolation(const float aColors[vKeyPoints][vLamps], int aDelay = 100)
{
  for (int vIndex = 0; vIndex < 1000; vIndex++) {
    float vBrightness = (float)vIndex / 1000.0f;
    SetLamps(aColors, vBrightness);
    delay(aDelay);
  }
  for (int vIndex = 0; vIndex < 1000; vIndex++) {
    float vBrightness = 1.0f - (float)vIndex / 1000.0f;
    SetLamps(aColors, vBrightness);
    delay(aDelay);
  }
}

int mBrightness = 50;

ggLCD mLCD(0x27);
ggLCD mLCD2(0x26);

ggJoyStick mJoyStick(A0, A1, A2);

void BrightnessPrint()
{
  mLCD.PrintValue("Brightness", mBrightness, "%", 0, 0, 16);
  mLCD.PrintProgressBar(mBrightness, 0, 100, 0, 16, 20);
}

void Dimmer()
{
  static long vMicros = 0;
  long vMicrosNew = micros();
  if (vMicrosNew - vMicros > 100000) {
    int vValueA = analogRead(A0);
    int vValueB = analogRead(A1);
    int vValueC = analogRead(A2);
    if ((vValueA < 250) || (750 < vValueA)) {
      if ((vValueA < 250) && (mBrightness < 100)) mBrightness++;
      if ((vValueA > 750) && (mBrightness > 0)) mBrightness--;
      SetLamps(vAmbianceDimmer, 0.01f * mBrightness);
      BrightnessPrint();
    }
    
    mLCD.PrintValue("X", mJoyStick.GetX(), 1, 0, 10);
    mLCD.PrintValue("Y", mJoyStick.GetY(), 2, 0, 10);
    mLCD.PrintValue("S", mJoyStick.GetSwitch(), 3, 0, 10);
    mLCD.PrintProgressBar(100*mJoyStick.GetX(), -100, 100, 1, 10, 20);
    mLCD.PrintProgressBar(100*mJoyStick.GetY(), -100, 100, 2, 10, 20);
    mLCD.PrintProgressBar(mJoyStick.GetSwitch(), 0, 1, 3, 10, 20);

    mLCD2.PrintValue("A0", analogRead(A0), 0, 0, 10);
    mLCD2.PrintValue("A1", analogRead(A1), 1, 0, 10);
    mLCD2.PrintValue("A2", analogRead(A2), 2, 0, 10);
    mLCD2.PrintValue("A3", analogRead(A3), 0, 10, 20);
    mLCD2.PrintValue("A4", analogRead(A4), 1, 10, 20);
    mLCD2.PrintValue("A5", analogRead(A5), 2, 10, 20);

    vMicros = vMicrosNew;
  }
}

void setup()
{
  // Serial.begin(115200);
  for (int vLamp = 0; vLamp < vLamps; vLamp++) {
    pinMode(vLampsPin[vLamp], OUTPUT);
    analogWrite(vLampsPin[vLamp], 0);
  }
  pinMode(13, OUTPUT);
  
  mLCD.begin();
  mLCD2.begin();
  
  mJoyStick.begin();
  mJoyStick.CalibrateX(480, 2, 1018);
  mJoyStick.CalibrateY(524, 2, 1019);
  mJoyStick.CalibrateSwitch(false);

  SetLamps(vAmbianceDimmer, 0.01f * mBrightness);
  
  BrightnessPrint();
}

void loop()
{
  // TestCalibration();
  // TestKeyPoints(vRainbowA);
  // TestInterpolation(vAmbianceDimmer, 3);
  // SetLamps(vAmbianceDimmer, 0.5f);
  Dimmer();
}

