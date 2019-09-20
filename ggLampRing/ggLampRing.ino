#include <FastLED.h>
#include "ggSampler.h"


#define M_LED_DATA_PIN 13
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


ggSampler mSamplerR(17.0f);
ggSampler mSamplerG(19.0f);
ggSampler mSamplerB(27.0f);
// ggSampler mSamplerPattern(0.01f);


void setup()
{
  SetAll(CRGB(0, 0, 0));
  FastLED.addLeds<WS2811, M_LED_DATA_PIN, RGB>(mLEDs, M_NUM_LEDS);
  // FastLED.setCorrection(CRGB(150, 255, 180)); // no cover (red and blue are dominant, need to be reduced)
  FastLED.setCorrection(CRGB(255, 190, 170)); // blue-greenish glass cover (blue and green are too dominant)
  FastLED.setBrightness(64);
  FastLED.show();

  const int vDelay = 1000;

  SetAll(CRGB(0, 0, 0));
  FastLED.show();
  delay(vDelay);

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

  SetAll(CRGB(255, 255, 255));
  FastLED.show();
  delay(vDelay);

  SetRainbow();
  FastLED.show();
  delay(vDelay);
/*
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
  /*
  mSamplerPattern.OnSample([] () {
    static bool vFlipFlop = true;
    vFlipFlop ? SetRainbow() : SetArcs();
    vFlipFlop = !vFlipFlop;
    FastLED.show();
  });
  */
}


void loop()
{
  mSamplerR.Run();
  mSamplerG.Run();
  mSamplerB.Run();
  // mSamplerPattern.Run();
}
