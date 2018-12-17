#include <FastLED.h>

#define LED_PIN_1 12
#define LED_PIN_2 13

#define M_NUM_LEDS 64

CRGB mLEDs1[M_NUM_LEDS];
CRGB mLEDs2[M_NUM_LEDS];

void ShowColors(const CRGB& aRGB1,
                const CRGB& aRGB2)
{
  for (int vIndex = 0; vIndex < M_NUM_LEDS; vIndex++) {
    mLEDs1[vIndex] = aRGB1;
    mLEDs2[vIndex] = aRGB2;
  }
  FastLED.show();
}

void AnimateRed()
{
  static int vValueMax = 50;
  static int vValue = 0;
  static int vDelta = 1;
  if (vValue <= 0) vDelta = 1;
  if (vValue >= vValueMax) vDelta = -1;
  vValue += vDelta;
  ShowColors(CRGB(2*vValueMax, vValueMax-vValue, vValue),
             CRGB(2*vValueMax, vValue, vValueMax-vValue));
}

void setup()
{ 
  FastLED.addLeds<WS2811, LED_PIN_1, RGB>(mLEDs1, M_NUM_LEDS);
  FastLED.addLeds<WS2811, LED_PIN_2, RGB>(mLEDs2, M_NUM_LEDS);
  //ShowColors(CRGB(0,100,20),CRGB(0,0,100)); // warm orange
}

void loop()
{
  AnimateRed();
  //delay(1000);
}
