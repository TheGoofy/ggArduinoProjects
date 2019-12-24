#include <FastLED.h>


template <int TPin, int TNumLEDs = 24>
class ggLEDRing {

public:

  ggLEDRing()
  : mUpdateOutputStart(nullptr),
    mUpdateOutputFinish(nullptr),
    mOn(false),
    mHSV(CHSV(200,255,100)) {
  }

  void Begin() {
    // Print(Serial);
    FastLED.addLeds<WS2811, TPin, RGB>(mLEDs, TNumLEDs);
    // FastLED.setCorrection(CRGB(150, 255, 180)); // no cover (red and blue are dominant, need to be reduced)
    // FastLED.setCorrection(CRGB(255, 190, 170)); // blue-greenish glass cover (blue and green are too dominant)
    // FastLED.setBrightness(255);
    Set(mHSV.Get());
    UpdateOutput();
  }

  void Set(int aIndex, const CRGB& aRGB) {
    mLEDs[aIndex % TNumLEDs] = aRGB;
  }

  void Set(int aIndexBegin, int aCount, const CRGB& aRGB) {
    for (int vIndex = aIndexBegin; vIndex < aIndexBegin + aCount; vIndex++) {
      mLEDs[vIndex % TNumLEDs] = aRGB;
    }
  }

  void Set(const CRGB& aRGB) {
    Set(0, TNumLEDs, aRGB);
  }

  bool GetOn() const {
    return mOn;
  }

  void SetOn(bool aOn) {
    if (mOn != aOn) {
      mOn = aOn;
      UpdateOutput();
    }
  }

  void ToggleOnOff() {
    SetOn(!GetOn());
  }

  void ChangeChannel(int aChannel, int aDelta) {
    if (!GetOn()) return;
    CHSV vHSV = mHSV.Get();
    int vValue = vHSV.raw[aChannel];
    switch (aChannel) {
      case 0: vValue = (vValue + aDelta) & 0xff; break;
      case 1: vValue = ggClamp<int>(vValue - 2 * aDelta, 0, 255); break;
      case 2: vValue = ggClamp<int>(vValue + aDelta, 0, 255); break;
    }
    vHSV.raw[aChannel] = vValue;
    mHSV.Set(vHSV);
    UpdateOutput();
  }

  void ShowChannel(int aChannel) {
    FillChannel(aChannel);
    if (mUpdateOutputStart != nullptr) mUpdateOutputStart();
    FastLED.show();
    if (mUpdateOutputFinish != nullptr) mUpdateOutputFinish();
  }

  typedef std::function<void()> tEventFunc;
  
  void OnUpdateOutputStart(tEventFunc aUpdateOutputStart) {
    mUpdateOutputStart = aUpdateOutputStart;
  }

  void OnUpdateOutputFinish(tEventFunc aUpdateOutputFinish) {
    mUpdateOutputFinish = aUpdateOutputFinish;
  }

private:

  template <typename TStream>
  void Print(TStream& aStream) {
    aStream.printf("%s - mOn=%d mHSV=%d/%d/%d\n", __PRETTY_FUNCTION__, mOn, mHSV.Get().h, mHSV.Get().s, mHSV.Get().v);
    for (int vIndex = 0; vIndex < TNumLEDs; vIndex++) {
      const CRGB& vRGB = mLEDs[vIndex];
      aStream.printf("mLEDs[%d]=%d/%d/%d ", vIndex, vRGB.r, vRGB.g, vRGB.b);
      if (vIndex % 6 == 5) aStream.println();
    }
    aStream.flush();
  }

  void FillChannel(int aChannel) {
    CHSV vHSV = mHSV;
    // if (aChannel == 0) vHSV.s = 255;
    if ((aChannel != 2) && (vHSV.v < 128)) vHSV.v = 128;
    for (int vIndex = 0; vIndex < TNumLEDs; vIndex++) {
      vHSV.raw[aChannel] = 256 * (vIndex + 1) / TNumLEDs - 1;
      mLEDs[(vIndex + 3 * TNumLEDs / 4) % TNumLEDs] = vHSV;
    }
  }

  void UpdateOutput() {
    Print(Serial);
    if (GetOn()) {
      CRGB vRGB;
      hsv2rgb_rainbow(mHSV.Get(), vRGB);
      Set(vRGB);
    }
    else {
      Set(CRGB::Black);
    }
    if (mUpdateOutputStart != nullptr) mUpdateOutputStart();
    FastLED.show();
    if (mUpdateOutputFinish != nullptr) mUpdateOutputFinish();
  }

  // basic setup
  CRGB mLEDs[TNumLEDs];
  tEventFunc mUpdateOutputStart;
  tEventFunc mUpdateOutputFinish;
  bool mOn;
  
  // persistent settings
  // ggValueEEPromT<CRGB> mRGB;
  ggValueEEPromT<CHSV> mHSV;
  
};
