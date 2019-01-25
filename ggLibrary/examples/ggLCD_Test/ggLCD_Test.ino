#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "ggLCD.h"

ggLCD mLCD(0x27);

void setup()
{
  mLCD.begin();
  mLCD.PrintValue("hello", 42, 0, 11);
  mLCD.PrintValue("goofy", 99, 3,  0);
}

void loop()
{
  float vX = 100.0f * sin(0.001f * millis());
  float vY = 100.0f * cos(0.001f * millis());
  mLCD.PrintProgressBar( vX, -100, 100, 0,  0, 10);
  mLCD.PrintProgressBar( vY, -100, 100, 1,  3, 13);
  mLCD.PrintProgressBar(-vX, -100, 100, 2,  7, 17);
  mLCD.PrintProgressBar(-vY, -100, 100, 3, 10, 20);
  delay(100);
}
