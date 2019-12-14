#include <FastLED.h>

template <int TPin, int TNumLEDs = 24>
class ggLEDRing {

public:

  ggLEDRing()
  /*: mOn(false)*/ {
  }

  void Begin() {
    FastLED.addLeds<WS2811, TPin, RGB>(mLEDs, TNumLEDs);
    // FastLED.setCorrection(CRGB(150, 255, 180)); // no cover (red and blue are dominant, need to be reduced)
    // FastLED.setCorrection(CRGB(255, 190, 170)); // blue-greenish glass cover (blue and green are too dominant)
    FastLED.setBrightness(255);
    Set(CRGB(255,0,0));
    UpdateOutput();
  }

  void Set(int aIndex, const CRGB& aRGB) {
    mLEDs[aIndex % TNumLEDs] = aRGB;
    UpdateOutput();
  }

  void Set(int aIndexBegin, int aCount, const CRGB& aRGB) {
    for (int vIndex = aIndexBegin; vIndex < aIndexBegin + aCount; vIndex++) {
      mLEDs[vIndex % TNumLEDs] = aRGB;
    }
    UpdateOutput();
  }

  void Set(const CRGB& aRGB) {
    Set(0, TNumLEDs, aRGB);
  }

  void SetOn(bool aOn) {
    // mOn.Set(aOn);
    UpdateOutput();
  }

private:

  void UpdateOutput() {
    FastLED.show();
  }

  // basic settings
  CRGB mLEDs[TNumLEDs];
  
  // persistent settings
  // ggValueEEPromT<bool> mOn;
  
};
