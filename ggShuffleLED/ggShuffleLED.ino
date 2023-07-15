// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// Released under the GPLv3 license to match the rest of the
// Adafruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#include "ggArrayShuffled.h"


// Which pin on the Arduino is connected to the NeoPixels?
#define M_PIN 2 // On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define M_NUM_PIXELS 64 // Popular NeoPixel ring size
#define M_NUM_COLORS 10

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel mPixels(M_NUM_PIXELS, M_PIN, NEO_RGB + NEO_KHZ800);
std::array<uint8_t, M_NUM_PIXELS> mPixelColorIndices{0};

std::array<uint32_t, M_NUM_COLORS> mColors = {
  Adafruit_NeoPixel::Color(35, 0, 100),
  Adafruit_NeoPixel::Color(25, 0, 200),
  Adafruit_NeoPixel::Color(35, 100, 255),
  Adafruit_NeoPixel::Color(50, 200, 150),
  Adafruit_NeoPixel::Color(150, 220, 50),
  Adafruit_NeoPixel::Color(255, 220, 20),
  Adafruit_NeoPixel::Color(255, 130, 20),
  Adafruit_NeoPixel::Color(255, 0, 30),
  Adafruit_NeoPixel::Color(100, 0, 40),
  Adafruit_NeoPixel::Color(50, 0, 60),
};

ggArrayShuffled<uint32_t, M_NUM_COLORS * M_NUM_PIXELS> mNumbers(0);


void setup()
{
  mPixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  mPixels.setBrightness(32);
}


void loop()
{
  for (uint32_t vNumber : mNumbers) {
    uint32_t vPixelIndex = vNumber % M_NUM_PIXELS;
    uint8_t vColorIndex = mPixelColorIndices[vPixelIndex];
    mPixels.setPixelColor(vPixelIndex, mColors[vColorIndex]);
    mPixelColorIndices[vPixelIndex] = (vColorIndex + 1) % M_NUM_COLORS;
    mPixels.show();
    delay(10);
  }

  mNumbers.Shuffle();
}
