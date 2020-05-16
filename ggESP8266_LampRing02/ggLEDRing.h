#include <Adafruit_NeoPixel.h>
#include "ggColor.h"


template <int TNumLEDs = 64>
class ggLEDRing {

public:

  ggLEDRing(int aPinA, int aPinB)
  : mLEDsA(TNumLEDs, aPinA, NEO_BGR + NEO_KHZ800),
    mLEDsB(TNumLEDs, aPinB, NEO_BGR + NEO_KHZ800),
    mFuncShowStart(nullptr),
    mFuncShowFinish(nullptr),
    mOn(false),
    mHSV(ggColor::cHSV(10,255,100)) {
  }

  void Begin() {
    // Print(Serial);
    mLEDsA.begin();
    mLEDsB.begin();
    UpdateOutput();
  }

  void ResetSettings() {
    SetColor(ggColor::cHSV(10,255,100));
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

  const ggColor::cHSV& GetColorHSV() const {
    return mHSV.Get();
  }

  ggColor::cRGB GetColorRGB() const {
    return ggColor::ToRGB(mHSV);
  }

  void SetColor(const ggColor::cHSV& aHSV) {
    if (mHSV.Get() != aHSV) {
      mHSV = aHSV;
      UpdateOutput();
    }
  }

  void SetColor(const ggColor::cRGB& aRGB) {
    SetColor(ggColor::ToHSV(aRGB));
  }

  void ChangeChannel(int aChannel, int aDelta) {
    if (!GetOn()) return;
    ggColor::cHSV vHSV = mHSV;
    int vValue = vHSV.mChannels[aChannel];
    switch (aChannel) {
      case 0: vValue = (vValue + aDelta) & 0xff; break;
      case 1: vValue = ggClamp<int>(vValue - 2 * aDelta, 0, 255); break;
      case 2: vValue = ggClamp<int>(vValue + aDelta, 0, 255); break;
    }
    vHSV.mChannels[aChannel] = vValue;
    if (mHSV.Get() != vHSV) {
      mHSV = vHSV;
      UpdateOutput();
    }
  }

  void DisplayChannel(int aChannel) {
    ggColor::cHSV vHSV = mHSV;
    if ((aChannel != 1) && (vHSV.mS < 128)) vHSV.mS = 128;
    if ((aChannel != 2) && (vHSV.mV < 128)) vHSV.mV = 128;
    for (int vIndex = 0; vIndex < TNumLEDs; vIndex++) {
      vHSV.mChannels[aChannel] = 256 * (vIndex + 1) / TNumLEDs - 1;
      mLEDsA.setPixelColor((vIndex + 0 * TNumLEDs / 4) % TNumLEDs, ggColor::ToRGB(vHSV));
      mLEDsB.setPixelColor((vIndex + 0 * TNumLEDs / 4) % TNumLEDs, ggColor::ToRGB(vHSV));
    }
    Show();
  }

  void DisplayProgress(float aProgress, const ggColor::cRGB& aColor, const ggColor::cRGB& aBackground) {
    int vProgress = TNumLEDs * aProgress + 0.5f;
    mLEDsA.fill(aColor, 0, vProgress);
    mLEDsA.fill(aBackground, vProgress, TNumLEDs - vProgress);
    mLEDsB.fill(aColor, 0, vProgress);
    mLEDsB.fill(aBackground, vProgress, TNumLEDs - vProgress);
    Show();
  }

  typedef std::function<void()> tFunc;
  
  void OnShowStart(tFunc aFuncShowStart) {
    mFuncShowStart = aFuncShowStart;
  }

  void OnShowFinish(tFunc aFuncShowFinish) {
    mFuncShowFinish = aFuncShowFinish;
  }

  void DisplayNormal() {
    UpdateOutput();
  }

private:

  template <typename TStream>
  void Print(TStream& aStream) {
    aStream.printf("%s - mOn=%d mHSV=%d/%d/%d\n", __PRETTY_FUNCTION__, mOn, mHSV.Get().mH, mHSV.Get().mS, mHSV.Get().mV);
    for (int vIndex = 0; vIndex < TNumLEDs; vIndex++) {
      ggColor::cRGB vRGB(mLEDsA.getPixelColor(vIndex));
      aStream.printf("mLEDsA[%d]=%d/%d/%d ", vIndex, vRGB.mR, vRGB.mG, vRGB.mB);
      if (vIndex % 6 == 5) aStream.println();
    }
    aStream.flush();
  }
   
  void Show() {
    if (mFuncShowStart != nullptr) mFuncShowStart();
    mLEDsA.show();
    mLEDsB.show();
    if (mFuncShowFinish != nullptr) mFuncShowFinish();
  }

  void UpdateOutput() {
    // Print(Serial);
    if (GetOn()) {
      mLEDsA.fill(ggColor::ToRGB(mHSV.Get()));
      mLEDsB.fill(ggColor::ToRGB(mHSV.Get()));
    }
    else {
      mLEDsA.fill(ggColor::cRGB::Black());
      mLEDsB.fill(ggColor::cRGB::Black());
    }
    Show();
  }

  // basic setup
  Adafruit_NeoPixel mLEDsA;
  Adafruit_NeoPixel mLEDsB;
  tFunc mFuncShowStart;
  tFunc mFuncShowFinish;
  bool mOn;
  
  // persistent settings
  ggValueEEPromT<ggColor::cHSV> mHSV;
  
};
