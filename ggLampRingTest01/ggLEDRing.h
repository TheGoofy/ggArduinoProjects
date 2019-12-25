#include <Adafruit_NeoPixel.h>
#include "ggColor.h"


template <int TPin, int TNumLEDs = 24>
class ggLEDRing {

public:

  ggLEDRing()
  : mLEDs(TNumLEDs, TPin, NEO_RGB + NEO_KHZ800),
    mUpdateOutputStart(nullptr),
    mUpdateOutputFinish(nullptr),
    mOn(false),
    mHSV(ggColor::cHSV(200,255,100)) {
  }

  void Begin() {
    // Print(Serial);
    mLEDs.begin();
    // FastLED.setCorrection(CRGB(150, 255, 180)); // no cover (red and blue are dominant, need to be reduced)
    // FastLED.setCorrection(CRGB(255, 190, 170)); // blue-greenish glass cover (blue and green are too dominant)
    // FastLED.setBrightness(255);
    UpdateOutput();
  }

  void Set(const ggColor::cRGB& aRGB) {
    mLEDs.fill(aRGB);
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
    ggColor::cHSV vHSV = mHSV.Get();
    int vValue = vHSV.mChannels[aChannel];
    switch (aChannel) {
      case 0: vValue = (vValue + aDelta / 2) & 0xff; break;
      case 1: vValue = ggClamp<int>(vValue - 2 * aDelta, 0, 255); break;
      case 2: vValue = ggClamp<int>(vValue + aDelta, 0, 255); break;
    }
    vHSV.mChannels[aChannel] = vValue;
    mHSV.Set(vHSV);
    UpdateOutput();
  }

  void ShowChannel(int aChannel) {
    FillChannel(aChannel);
    if (mUpdateOutputStart != nullptr) mUpdateOutputStart();
    mLEDs.show();
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
    aStream.printf("%s - mOn=%d mHSV=%d/%d/%d\n", __PRETTY_FUNCTION__, mOn, mHSV.Get().mH, mHSV.Get().mS, mHSV.Get().mV);
    for (int vIndex = 0; vIndex < TNumLEDs; vIndex++) {
      ggColor::cRGB vRGB(mLEDs.getPixelColor(vIndex));
      aStream.printf("mLEDs[%d]=%d/%d/%d ", vIndex, vRGB.mR, vRGB.mG, vRGB.mB);
      if (vIndex % 6 == 5) aStream.println();
    }
    aStream.flush();
  }
   
  void FillChannel(int aChannel) {
    ggColor::cHSV vHSV = mHSV;
    if ((aChannel != 1) && (vHSV.mS < 128)) vHSV.mS = 128;
    if ((aChannel != 2) && (vHSV.mV < 128)) vHSV.mV = 128;
    for (int vIndex = 0; vIndex < TNumLEDs; vIndex++) {
      vHSV.mChannels[aChannel] = 256 * (vIndex + 1) / TNumLEDs - 1;
      mLEDs.setPixelColor((vIndex + 3 * TNumLEDs / 4) % TNumLEDs, ToRGB(vHSV));
    }
  }

  void UpdateOutput() {
    // Print(Serial);
    if (GetOn()) {
      Set(ggColor::ToRGB(mHSV.Get()));
    }
    else {
      Set(ggColor::cRGB::Black());
    }
    if (mUpdateOutputStart != nullptr) mUpdateOutputStart();
    mLEDs.show();
    if (mUpdateOutputFinish != nullptr) mUpdateOutputFinish();
  }

  // basic setup
  Adafruit_NeoPixel mLEDs;
  tEventFunc mUpdateOutputStart;
  tEventFunc mUpdateOutputFinish;
  bool mOn;
  
  // persistent settings
  ggValueEEPromT<ggColor::cHSV> mHSV;
  
};
