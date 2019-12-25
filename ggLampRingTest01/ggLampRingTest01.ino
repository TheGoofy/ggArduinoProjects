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
    eRingChannel0,
    eRingChannel1,
    eRingChannel2
  };
  
  static tEnum Toggle(tEnum aMode) {
    switch (aMode) {
      case eCenter: return eRingChannel0;
      case eRingChannel0: return eRingChannel1;
      case eRingChannel1: return eRingChannel2;
      case eRingChannel2: return eCenter;
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
      if (vMode == ggMode::eCenter) Periphery().ToggleOnOff();
      else vMode = ggMode::eCenter;
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
      case ggMode::eCenter: break;
      case ggMode::eRingChannel0: Periphery().mLEDRing.DisplayChannel(0); break;
      case ggMode::eRingChannel1: Periphery().mLEDRing.DisplayChannel(1); break;
      case ggMode::eRingChannel2: Periphery().mLEDRing.DisplayChannel(2); break;
    }
  });

  // rotary encoder signal
  Periphery().mEncoder.OnValueChangedDelta([&] (long aValueDelta) {
    if (!Periphery().mOn.Get()) return;
    // encoder has 4 increments per tick and 20 ticks per revolution, one revolution is 100%
    switch (vMode) {
      case ggMode::eCenter: Periphery().mLEDCenter.ChangeBrightness(0.25f * 0.05f * aValueDelta); break;
      case ggMode::eRingChannel0: Periphery().mLEDRing.ChangeChannel(0, aValueDelta); break;
      case ggMode::eRingChannel1: Periphery().mLEDRing.ChangeChannel(1, aValueDelta); break;
      case ggMode::eRingChannel2: Periphery().mLEDRing.ChangeChannel(2, aValueDelta); break;
    }
  });

  // switch off pwm during ws2811 serial communication (timing scrambled)
  Periphery().mLEDRing.OnShowStart([&] () {
    Periphery().mLEDCenter.Stop();
  });
  Periphery().mLEDRing.OnShowFinish([&] () {
    Periphery().mLEDCenter.Resume();
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
