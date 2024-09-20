#pragma once

#include <Adafruit_NeoPixel.h>


// Fan LED control
#define M_LED_NUM_FANS 3
#define M_LED_NUM_LEDS_PER_FAN 12
#define M_LED_TOTAL (M_LED_NUM_FANS * M_LED_NUM_LEDS_PER_FAN)


#define BRIGHTNESS 255 // Set brightness level (0-255)


// Create an instance of the Adafruit_NeoPixel class
Adafruit_NeoPixel mFanLEDs = Adafruit_NeoPixel(M_LED_TOTAL, M_LED_DATA_PIN, NEO_GRB + NEO_KHZ800);


void SetupLED()
{
  // the LEDs of the fans
  mFanLEDs.begin();
  mFanLEDs.setBrightness(BRIGHTNESS);
  mFanLEDs.show(); // Initialize all pixels to 'off'
}


void ShowA(uint8_t aAngle)
{
  // static uint8_t vBrightness = 0;
  // mFanLEDs.setBrightness(vBrightness++);
  auto vColorA = mFanLEDs.Color(0, 0, 0);
  auto vColorB = mFanLEDs.Color(50, 0, 50);
  for (uint8_t vIndex = 0; vIndex < mFanLEDs.numPixels(); vIndex++) {
    mFanLEDs.setPixelColor(vIndex, (vIndex + aAngle) % 12 > 6 ? vColorA : vColorB);
  }
}


void TickLEDs()
{
  static uint8_t vAngle = 0;
  vAngle = (vAngle + 1) % 12;
  ShowA(vAngle);
  mFanLEDs.show();
}
