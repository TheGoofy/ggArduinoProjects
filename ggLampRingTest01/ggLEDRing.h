#include <FastLED.h>
#include <Ticker.h>


template <int TPin, int TNumLEDs = 24>
class ggLEDRing {

public:

  ggLEDRing()
  : mUpdateOutputStart(nullptr),
    mUpdateOutputFinish(nullptr),
    mTickCount(0),
    mOn(false),
    mRGB(CRGB(200,50,0)) {
  }

  void Begin() {
    // Print(Serial);
    FastLED.addLeds<WS2811, TPin, RGB>(mLEDs, TNumLEDs);
    // FastLED.setCorrection(CRGB(150, 255, 180)); // no cover (red and blue are dominant, need to be reduced)
    // FastLED.setCorrection(CRGB(255, 190, 170)); // blue-greenish glass cover (blue and green are too dominant)
    // FastLED.setBrightness(255);
    Set(mRGB.Get());
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

  void ChangeColor(int aChannel, int aDelta) {
    if (!GetOn()) return;
    if (mTicker.active()) mTicker.detach();
    CRGB vRGB = mRGB.Get();
    int vValue = vRGB[aChannel];
    vValue = ggClamp<int>(vValue + aDelta, 0, 255);
    vRGB[aChannel] = vValue;
    mRGB.Set(vRGB);
    UpdateOutput();
  }

  void Blink(const CRGB& aRGB) {
    mTickCount = 0;
    mBlinkColor = aRGB;
    mTicker.attach_ms(250, [&] () {
      if (mTickCount < 6) {
        Set(mTickCount % 2 == 0 ? mBlinkColor : mRGB.Get());
        if (mUpdateOutputStart != nullptr) mUpdateOutputStart();
        FastLED.show();
        if (mUpdateOutputFinish != nullptr) mUpdateOutputFinish();
        mTickCount++;
      }
      else {
        mTicker.detach();
      }
    });
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
    aStream.printf("%s - mOn=%d mRGB=%d/%d/%d\n", __PRETTY_FUNCTION__, mOn, mRGB.Get().r, mRGB.Get().g, mRGB.Get().b);
    for (int vIndex = 0; vIndex < TNumLEDs; vIndex++) {
      const CRGB& vRGB = mLEDs[vIndex];
      aStream.printf("mLEDs[%d]=%d/%d/%d ", vIndex, vRGB.r, vRGB.g, vRGB.b);
      if (vIndex % 6 == 5) aStream.println();
    }
    aStream.flush();
  }

  void UpdateOutput() {
    // Print(Serial);
    Set(GetOn() ? mRGB.Get() : CRGB::Black);
    if (mUpdateOutputStart != nullptr) mUpdateOutputStart();
    FastLED.show();
    if (mUpdateOutputFinish != nullptr) mUpdateOutputFinish();
  }

  // basic setup
  CRGB mLEDs[TNumLEDs];
  tEventFunc mUpdateOutputStart;
  tEventFunc mUpdateOutputFinish;
  Ticker mTicker;
  int mTickCount;
  CRGB mBlinkColor;
  bool mOn;
  
  // persistent settings
  ggValueEEPromT<CRGB> mRGB;
  
};
