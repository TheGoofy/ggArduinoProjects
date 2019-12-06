#include <FastLED.h>
#include "ggSampler.h"


#define M_LED_DATA_PIN 16
#define M_NUM_LEDS 24

CRGB mLEDs[M_NUM_LEDS];


void Set(int aIndexBegin, int aCount, const CRGB& aRGB)
{
  for (int vIndex = aIndexBegin; vIndex < aIndexBegin + aCount; vIndex++) {
    mLEDs[vIndex % M_NUM_LEDS] = aRGB;
  }
}


void SetAll(const CRGB& aRGB)
{
  Set(0, M_NUM_LEDS, aRGB);
}


void SetRainbow()
{
  for (int vIndex = 0; vIndex < M_NUM_LEDS; vIndex++) {
    float vAlpha = 2.0f * M_PI * vIndex / M_NUM_LEDS;
    float vBrightness = 0.5f + 0.5f * sin(vAlpha);
    vBrightness = pow(vBrightness, 1.5f);
    int vValue = 255 * vBrightness;
    if (vValue < 0) vValue = 0;
    if (vValue > 255) vValue = 255;
    int vIndexR = vIndex;
    int vIndexG = (vIndexR + M_NUM_LEDS/3) % M_NUM_LEDS;
    int vIndexB = (vIndexG + M_NUM_LEDS/3) % M_NUM_LEDS;
    mLEDs[vIndexR].red = vValue;
    mLEDs[vIndexG].green = vValue;
    mLEDs[vIndexB].blue = vValue;
  }
}


void SetArcs()
{
  const int vCount = 5;
  SetAll(CRGB::Black);
  Set( 0, vCount, CRGB(255, 0, 0));
  Set( 8, vCount, CRGB(0, 240, 0));
  Set(16, vCount, CRGB(0, 0, 210));
}


void ShiftLeft()
{
  CRGB vTempLED = mLEDs[0];
  for (int vIndex = 1; vIndex < M_NUM_LEDS; vIndex++) {
    mLEDs[vIndex - 1] = mLEDs[vIndex];
  }
  mLEDs[M_NUM_LEDS - 1] = vTempLED;
}


void ShiftLeftR()
{
  int vTempR = mLEDs[0].r;
  for (int vIndex = 1; vIndex < M_NUM_LEDS; vIndex++) {
    mLEDs[vIndex - 1].r = mLEDs[vIndex].r;
  }
  mLEDs[M_NUM_LEDS - 1].r = vTempR;
}


void ShiftLeftG()
{
  int vTempG = mLEDs[0].green;
  for (int vIndex = 1; vIndex < M_NUM_LEDS; vIndex++) {
    mLEDs[vIndex - 1].g = mLEDs[vIndex].g;
  }
  mLEDs[M_NUM_LEDS - 1].g = vTempG;
}


void ShiftLeftB()
{
  int vTempB = mLEDs[0].b;
  for (int vIndex = 1; vIndex < M_NUM_LEDS; vIndex++) {
    mLEDs[vIndex - 1].b = mLEDs[vIndex].b;
  }
  mLEDs[M_NUM_LEDS - 1].b = vTempB;
}


CRGB GetTemperatureColorRed(float aTemperature)
{
  CRGB vTemperatureColor;
  vTemperatureColor.r = 255;
  vTemperatureColor.g = (aTemperature < 0.7f) ? 255 * aTemperature / 0.7f : 255;
  vTemperatureColor.b = (aTemperature > 0.5f) ? 255 * (aTemperature - 0.5f) / 0.5f : 0;
  return vTemperatureColor;
}


CRGB GetTemperatureColorGreen(float aTemperature)
{
  CRGB vTemperatureColor;
  vTemperatureColor.r = (aTemperature < 0.5f) ? 255 * aTemperature / 0.5f : 255;
  vTemperatureColor.g = 255;
  vTemperatureColor.b = (aTemperature > 0.5f) ? 255 * (aTemperature - 0.5f) / 0.5f : 0;
  return vTemperatureColor;
}


CRGB GetTemperatureColorBlue(float aTemperature)
{
  CRGB vTemperatureColor;
  vTemperatureColor.r = (aTemperature < 0.5f) ? 255 * aTemperature / 0.5f : 255;
  vTemperatureColor.g = (aTemperature > 0.5f) ? 255 * (aTemperature - 0.5f) / 0.5f : 0;
  vTemperatureColor.b = 255;
  return vTemperatureColor;
}


template <typename TTemperatureFunc>
void SetTemperature(TTemperatureFunc aTemperatureFunc)
{
  for (int vIndex = 0; vIndex < M_NUM_LEDS; vIndex++) {
    float vTemperature = (float)vIndex / (float)(M_NUM_LEDS - 1);
    mLEDs[vIndex] = aTemperatureFunc(vTemperature);
  }
}


template <typename TValueType>
const TValueType& ggClamp(const TValueType& aValue, const TValueType& aValueMin, const TValueType& aValueMax)
{
  return (aValue < aValueMin) ? aValueMin : (aValue > aValueMax ? aValueMax : aValue);
}


ggSampler mSamplerR(17.0f);
ggSampler mSamplerG(19.0f);
ggSampler mSamplerB(27.0f);
ggSampler mSamplerRGB(8.0f);
ggSampler mSamplerPattern(0.1f);
ggSampler mSamplerBrightness(10.0f);
ggSampler mSamplerTemperature(1.0f);


void setup()
{
  SetAll(CRGB(0, 0, 0));
  FastLED.addLeds<WS2811, M_LED_DATA_PIN, RGB>(mLEDs, M_NUM_LEDS);
  // FastLED.setCorrection(CRGB(150, 255, 180)); // no cover (red and blue are dominant, need to be reduced)
  FastLED.setCorrection(CRGB(255, 190, 170)); // blue-greenish glass cover (blue and green are too dominant)
  FastLED.setBrightness(255);
  FastLED.show();

  const int vDelay = 1000;

  SetAll(CRGB(0, 0, 0));
  FastLED.show();
  delay(vDelay);
/*
  SetAll(CRGB(255, 0, 0));
  FastLED.show();
  delay(vDelay);

  SetAll(CRGB(255, 255, 0));
  FastLED.show();
  delay(vDelay);

  SetAll(CRGB(0, 255, 0));
  FastLED.show();
  delay(vDelay);

  SetAll(CRGB(0, 255, 255));
  FastLED.show();
  delay(vDelay);

  SetAll(CRGB(0, 0, 255));
  FastLED.show();
  delay(vDelay);

  SetAll(CRGB(255, 0, 255));
  FastLED.show();
  delay(vDelay);
*/
  SetAll(CRGB(255, 255, 255));
  FastLED.show();
  delay(vDelay);
/*
  SetRainbow();
  FastLED.show();
  delay(vDelay);

  SetArcs();
  FastLED.show();
  delay(vDelay);
*/

  mSamplerR.OnSample([] () {
    ShiftLeftR();
    FastLED.show();
  });
  mSamplerG.OnSample([] () {
    ShiftLeftG();
    FastLED.show();
  });
  mSamplerB.OnSample([] () {
    ShiftLeftB();
    FastLED.show();
  });
  mSamplerRGB.OnSample([] () {
    ShiftLeft();
    FastLED.show();
  });
  
  mSamplerPattern.OnSample([] () {
    static int vPatternIndex = 0;
    switch (vPatternIndex) {
      case 0: SetTemperature(GetTemperatureColorRed); break;
      case 1: SetTemperature(GetTemperatureColorGreen); break;
      case 2: SetTemperature(GetTemperatureColorBlue); break;
      case 3: SetRainbow(); break;
      case 4: SetArcs(); break;
    }
    FastLED.show();
    vPatternIndex = (vPatternIndex + 1) % 5;
  });

  mSamplerBrightness.OnSample([] () {
    const float vBrightnessMin = 0.05f;
    const float vBrightnessMax = 1.00f;
    const float vBrightnessScale = 1.05f;
    static float vBrightness = 0.05f;
    static float vFactor = vBrightnessScale;
    vBrightness *= vFactor;
    if (vBrightness <= vBrightnessMin) {
      vFactor = vBrightnessScale;
      vBrightness = vBrightnessMin;
    }
    if (vBrightness >= vBrightnessMax) {
      vFactor = 1.0f/vBrightnessScale;
      vBrightness = vBrightnessMax;
    }
    vBrightness = ggClamp(vBrightness, vBrightnessMin, vBrightnessMax);
    FastLED.setBrightness(255 * vBrightness);
    FastLED.show();
  });

  mSamplerTemperature.OnSample([] () {
    const float vTemperatureMin = 0.0f;
    const float vTemperatureMax = 1.0f;
    static float vTemperature = 0.0f;
    static float vInc = 0.01f;
    vTemperature += vInc;
    if (vTemperature <= vTemperatureMin) vInc =  0.01f;
    if (vTemperature >= vTemperatureMax) vInc = -0.01f;
    vTemperature = ggClamp(vTemperature, vTemperatureMin, vTemperatureMax);
    SetAll(GetTemperatureColorRed(vTemperature));
    FastLED.show();
  });
}


void loop()
{
  // mSamplerR.Run();
  // mSamplerG.Run();
  // mSamplerB.Run();
  // mSamplerRGB.Run();
  // mSamplerPattern.Run();
  // mSamplerBrightness.Run();
  // mSamplerTemperature.Run();
  FastLED.setBrightness(255);
  FastLED.show();
  delay(2000);
  FastLED.setBrightness(127);
  FastLED.show();
  delay(2000);
  FastLED.setBrightness(63);
  FastLED.show();
  delay(2000);
  FastLED.setBrightness(31);
  FastLED.show();
  delay(2000);
}
