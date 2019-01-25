#include <FastLED.h>

#define LED_PIN_0 10
#define LED_PIN_1 11
#define LED_PIN_2 12
#define LED_PIN_3 13

#define M_NUM_LEDS 64

CRGB mLEDs0[M_NUM_LEDS];
CRGB mLEDs1[M_NUM_LEDS];
CRGB mLEDs2[M_NUM_LEDS];
CRGB mLEDs3[M_NUM_LEDS];

void ShowColors(const CRGB& aRGB0,
                const CRGB& aRGB1,
                const CRGB& aRGB2,
                const CRGB& aRGB3)
{
  for (int vIndex = 0; vIndex < M_NUM_LEDS; vIndex++) {
    mLEDs0[vIndex] = aRGB0;
    mLEDs1[vIndex] = aRGB1;
    mLEDs2[vIndex] = aRGB2;
    mLEDs3[vIndex] = aRGB3;
  }
  FastLED.show();
}

void AnimateRed(const int aValueMax)
{
  static int vValue = 0;
  static int vDelta = 1;
  if (vValue <= 0) vDelta = 1;
  if (vValue >= aValueMax) vDelta = -1;
  vValue += vDelta;
  CRGB vColorA(2*aValueMax, aValueMax-vValue, vValue);
  CRGB vColorB(2*aValueMax, vValue, aValueMax-vValue);
  ShowColors(vColorA, vColorB, vColorB, vColorA);
}

void AnimateGreen(const int aValueMax)
{
  static int vValue = 0;
  static int vDelta = 1;
  if (vValue <= 0) vDelta = 1;
  if (vValue >= aValueMax) vDelta = -1;
  vValue += vDelta;
  CRGB vColorA(vValue, 2*aValueMax, aValueMax-vValue);
  CRGB vColorB(aValueMax-vValue, 2*aValueMax, vValue);
  ShowColors(vColorA, vColorB, vColorB, vColorA);
}

void AnimateBlue(const int aValueMax)
{
  static int vValue = 0;
  static int vDelta = 1;
  if (vValue <= 0) vDelta = 1;
  if (vValue >= aValueMax) vDelta = -1;
  vValue += vDelta;
  CRGB vColorA(vValue, aValueMax-vValue, 2*aValueMax);
  CRGB vColorB(aValueMax-vValue, vValue, 2*aValueMax);
  ShowColors(vColorA, vColorB, vColorB, vColorA);
}

void setup()
{ 
  FastLED.addLeds<WS2811, LED_PIN_0, RGB>(mLEDs0, M_NUM_LEDS);
  FastLED.addLeds<WS2811, LED_PIN_1, RGB>(mLEDs1, M_NUM_LEDS);
  FastLED.addLeds<WS2811, LED_PIN_2, RGB>(mLEDs2, M_NUM_LEDS);
  FastLED.addLeds<WS2811, LED_PIN_3, RGB>(mLEDs3, M_NUM_LEDS);
  ShowColors(CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black);
}

void loop()
{
  const int vValueMax = 50;
  int vMode = (millis() / 10000) % 3;
  switch (vMode) {
    case 0: AnimateRed(vValueMax); break;
    case 1: AnimateGreen(vValueMax); break;
    case 2: AnimateBlue(vValueMax); break;
  }
  delay(50);
}
