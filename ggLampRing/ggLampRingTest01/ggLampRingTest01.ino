#define LED1_PWM_PIN 14
#define LED2_DATA_PIN 13
#define SW1_PIN A0
#define SW2_PIN 12

#include <FastLED.h>



CRGB mLEDs[24];
const int mNumLEDs = sizeof(mLEDs) / sizeof(CRGB);


bool SwitchDown()
{
  return analogRead(SW1_PIN) > 512;
}


bool SwitchUp()
{
  return digitalRead(SW2_PIN);
}


void Set(int aIndexBegin, int aCount, const CRGB& aRGB)
{
  for (int vIndex = aIndexBegin; vIndex < aIndexBegin + aCount; vIndex++) {
    mLEDs[vIndex % mNumLEDs] = aRGB;
  }
}


void SetAll(const CRGB& aRGB)
{
  Set(0, mNumLEDs, aRGB);
}


CRGB GetTemperatureColorRed(float aTemperature)
{
  CRGB vTemperatureColor;
  vTemperatureColor.r = 255;
  vTemperatureColor.g = (aTemperature < 0.7f) ? 255 * aTemperature / 0.7f : 255;
  vTemperatureColor.b = (aTemperature > 0.5f) ? 255 * (aTemperature - 0.5f) / 0.5f : 0;
  return vTemperatureColor;
}



template <typename TValueType>
const TValueType& ggClamp(const TValueType& aValue, const TValueType& aValueMin, const TValueType& aValueMax)
{
  return (aValue < aValueMin) ? aValueMin : (aValue > aValueMax ? aValueMax : aValue);
}


float ExponentialScale(float aValue, float aOffset = 2.0f, float aExponent = 8.7f)
{
  const float vMin = pow(aOffset, aExponent);
  const float vMax = pow(1.0f + aOffset, aExponent);
  return (pow(aValue + aOffset, aExponent) - vMin) / (vMax - vMin);
}


std::vector<int> mTable;


void setup()
{
  int vIndex = 0;
  int vStep = 1;
  while (vIndex < 1023) {
    mTable.push_back(vIndex);
    while (vIndex / vStep > 20) vStep++;
    vIndex += vStep;
  }
  mTable.push_back(1023);

  /*
  Serial.begin(115200);
  Serial.println();
  for (int vIndex = 0; vIndex < 1024; vIndex += 5) {
    float vBrightnessF = vIndex / 100.0f;
    int vBrightness1023 = 1023.0f * ExponentialScale(vBrightnessF) + 0.5f;
    Serial.printf("%d\t%d\n", vIndex, vBrightness1023);
  }
  Serial.flush();
  */

  pinMode(LED1_PWM_PIN, OUTPUT);
  analogWriteFreq(20000);
  analogWrite(LED1_PWM_PIN, 0);
  pinMode(LED2_DATA_PIN, OUTPUT);
  pinMode(SW1_PIN, INPUT_PULLUP);
  pinMode(SW2_PIN, INPUT_PULLUP);
  mLEDs[0] = CRGB::Red;
  mLEDs[1] = CRGB::Green;
  mLEDs[2] = CRGB::Blue;
  FastLED.addLeds<WS2811, LED2_DATA_PIN, RGB>(mLEDs, mNumLEDs);
  // FastLED.setCorrection(CRGB(150, 255, 180)); // no cover (red and blue are dominant, need to be reduced)
  // FastLED.setCorrection(CRGB(255, 190, 170)); // blue-greenish glass cover (blue and green are too dominant)
  FastLED.setBrightness(255);
  // SetAll(CRGB(255,150,60)); // almost white
  SetAll(CRGB(255,0,20));
  FastLED.show();
  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);
}


void loop()
{
  static int vBrightness = 0;
  
  if (SwitchUp()) vBrightness += 1;
  if (SwitchDown()) vBrightness -= 1;
  vBrightness = ggClamp<int>(vBrightness, 0, mTable.size() - 1);

  analogWrite(LED1_PWM_PIN, mTable[vBrightness]);

  digitalWrite(1, vBrightness > 0);
  digitalWrite(2, vBrightness < 100);
  
  delay(50);
}
