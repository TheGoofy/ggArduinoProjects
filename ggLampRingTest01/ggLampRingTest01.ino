#include "ggPeriphery.h"


ggPeriphery& Periphery()
{
  static ggPeriphery* vPeriphery = nullptr;
  if (vPeriphery == nullptr) vPeriphery = new ggPeriphery;
  return *vPeriphery;
}


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.flush();

  // create the periphery (before eeprom begins)
  Periphery();

  // startup eeprom utility class
  ggValueEEProm::Begin();
  
  // clicking "on/off" => center LED is master, ring just does the same
  Periphery().mButton.OnReleased([] () {
    Periphery().mLEDCenter.ToggleOnOff();
    Periphery().mLEDRing.SetOn(Periphery().mLEDCenter.GetOn());
  });

  // rotary encoder signal
  Periphery().mEncoder.OnValueChangedDelta([] (long aValueDelta) {
    // encoder has 4 increments per tick and 20 ticks per revolution, one revolution is 100%
    const float vValueDeltaPercent = 0.25f * 0.05f * aValueDelta;
    Periphery().mLEDCenter.ChangeBrightness(vValueDeltaPercent);
  });

  // setup connected hardware
  Periphery().Begin();
}


void loop()
{
  Periphery().Run();
}
