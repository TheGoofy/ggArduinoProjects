#include "ggPeriphery.h"


ggPeriphery mPeriphery;


void setup()
{
  Serial.begin(115200);
  Serial.println();

  // startup eeprom utility class
  Serial.printf("ggValueEEProm::Begin()\n");
  ggValueEEProm::Begin();
  
  // clicking "on/off" => center LED is master, ring just does the same
  mPeriphery.mButton.OnReleased([] () {
    mPeriphery.mLEDCenter.ToggleOnOff();
    mPeriphery.mLEDRing.SetOn(mPeriphery.mLEDCenter.GetOn());
  });

  // rotary encoder signal
  mPeriphery.mEncoder.OnValueChangedDelta([] (long aValueDelta) {
    Serial.printf("mPeriphery.mEncoder.OnValueChangedDelta\n");
    // encoder has 4 increments per tick and 20 ticks per revolution, one revolution is 100%
    const float vValueDeltaPercent = 0.25f * 0.05f * aValueDelta;
    mPeriphery.mLEDCenter.ChangeBrightness(vValueDeltaPercent);
  });

  // setup connected hardware
  Serial.printf("mPeriphery.Begin()\n");
  mPeriphery.Begin();
}


void loop()
{
  mPeriphery.Run();
}
