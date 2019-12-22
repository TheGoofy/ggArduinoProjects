#include "ggPeriphery.h"


ggPeriphery& Periphery()
{
  static ggPeriphery* vPeriphery = nullptr;
  if (vPeriphery == nullptr) vPeriphery = new ggPeriphery;
  return *vPeriphery;
}


struct ggMode {
  
  typedef enum tEnum {
    eCenter,
    eRingR,
    eRingG,
    eRingB
  };
  
  static tEnum Toggle(tEnum aMode) {
    switch (aMode) {
      case eCenter: return eRingR;
      case eRingR: return eRingG;
      case eRingG: return eRingB;
      case eRingB: return eCenter;
      default: return eCenter;
    }
  }
  
};


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.flush();

  // mode
  static ggMode::tEnum vMode = ggMode::eCenter;
  static bool vIgnoreNextReleasedEvent = false;
  
  // clicking "on/off"
  Periphery().mButton.OnReleased([&] () {
    if (!vIgnoreNextReleasedEvent) {
      Periphery().ToggleOnOff();
      vMode = ggMode::eCenter;
    }
    else {
      vIgnoreNextReleasedEvent = false;
    }
  });

  // long press changes mode
  Periphery().mButton.OnPressedFor(2000, [&] () {
    vIgnoreNextReleasedEvent = true;
    if (!Periphery().mOn.Get()) return;
    vMode = ggMode::Toggle(vMode);
    switch (vMode) {
      case ggMode::eCenter: Periphery().mLEDCenter.SetOn(true); break;
      case ggMode::eRingR: Periphery().mLEDCenter.SetOn(false); Periphery().mLEDRing.Blink(CRGB::Red); break;
      case ggMode::eRingG: Periphery().mLEDCenter.SetOn(false); Periphery().mLEDRing.Blink(CRGB::Green); break;
      case ggMode::eRingB: Periphery().mLEDCenter.SetOn(false); Periphery().mLEDRing.Blink(CRGB::Blue); break;
    }
  });

  // rotary encoder signal
  Periphery().mEncoder.OnValueChangedDelta([&] (long aValueDelta) {
    if (!Periphery().mOn.Get()) return;
    // encoder has 4 increments per tick and 20 ticks per revolution, one revolution is 100%
    switch (vMode) {
      case ggMode::eCenter: Periphery().mLEDCenter.ChangeBrightness(0.25f * 0.05f * aValueDelta); break;
      case ggMode::eRingR: Periphery().mLEDRing.ChangeColor(0, aValueDelta); break;
      case ggMode::eRingG: Periphery().mLEDRing.ChangeColor(1, aValueDelta); break;
      case ggMode::eRingB: Periphery().mLEDRing.ChangeColor(2, aValueDelta); break;
    }
  });

  // startup eeprom utility class
  ggValueEEProm::Begin();

  // initialize connected hardware
  Periphery().Begin();
}


void loop()
{
  Periphery().Run();
}
