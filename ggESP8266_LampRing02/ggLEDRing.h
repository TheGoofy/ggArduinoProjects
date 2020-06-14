#include <Adafruit_NeoPixel.h>
#include "ggColor.h"

enum class ggLocations {
  eBackBase  = 0b00000001,
  eBackTop   = 0b00000010,
  eFrontBase = 0b00000100,
  eFrontTop  = 0b00001000,
  eBack      = eBackBase  | eBackTop,
  eFront     = eFrontBase | eFrontTop,
  eBase      = eBackBase  | eFrontBase,
  eTop       = eBackTop   | eFrontTop,
  eAll       = eBack | eFront,
};

template <uint16_t TNumLEDs = 64>
class ggLEDRing {

public:

  ggLEDRing(int aPinA, int aPinB)
  : mLEDsA(TNumLEDs, aPinA, NEO_BGR + NEO_KHZ800),
    mLEDsB(TNumLEDs, aPinB, NEO_BGR + NEO_KHZ800),
    mOn(false),
    mHSV({ggColor::cHSV::DarkOrange(),
          ggColor::cHSV::DarkBlue(),
          ggColor::cHSV::DarkOrange(),
          ggColor::cHSV::DarkBlue()}) {
  }

  void Begin() {
    // Print(Serial);
    mLEDsA.begin();
    mLEDsB.begin();
    UpdateOutput();
  }

  void ResetSettings() {
    ggValueEEProm::cLazyWriter vLazyWriter;
    SetColor(ggColor::cHSV::DarkOrange(), ggLocations::eBase);
    SetColor(ggColor::cHSV::DarkBlue(), ggLocations::eTop);
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

  const ggColor::cHSV& GetColorHSV(ggLocations aLocations) const {
    return GetColor(GetLocationIndex(aLocations));
  }

  ggColor::cRGB GetColorRGB(ggLocations aLocations) const {
    return ggColor::ToRGB(GetColorHSV(aLocations));
  }

  void SetColor(const ggColor::cHSV& aHSV, ggLocations aLocations = ggLocations::eAll) {
    bool vColorChanged = false;
    ggValueEEProm::cLazyWriter vLazyWriter;
    SetColor(aHSV, GetLocationIndex(aLocations, ggLocations::eBackBase),  vColorChanged);
    SetColor(aHSV, GetLocationIndex(aLocations, ggLocations::eBackTop),   vColorChanged);
    SetColor(aHSV, GetLocationIndex(aLocations, ggLocations::eFrontBase), vColorChanged);
    SetColor(aHSV, GetLocationIndex(aLocations, ggLocations::eFrontTop),  vColorChanged);
    if (vColorChanged) UpdateOutput();
  }

  void SetColor(const ggColor::cRGB& aRGB, ggLocations aLocations = ggLocations::eAll) {
    SetColor(ggColor::ToHSV(aRGB), aLocations);
  }

  void ChangeColorChannel(int aChannel, int aDelta, ggLocations aLocations = ggLocations::eAll) {
    if (!GetOn()) return;
    ggColor::cHSV vHSV = mHSV[0];
    int vValue = vHSV.mChannels[aChannel];
    switch (aChannel) {
      case 0: vValue = (vValue + aDelta) & 0xff; break;
      case 1: vValue = ggClamp<int>(vValue - 2 * aDelta, 0, 255); break;
      case 2: vValue = ggClamp<int>(vValue + aDelta, 0, 255); break;
    }
    vHSV.mChannels[aChannel] = vValue;
    SetColor(vHSV, aLocations);
  }

  void DisplayColor(const ggColor::cRGB& aColor) {
    mLEDsA.fill(aColor, 0, TNumLEDs);
    mLEDsB.fill(aColor, 0, TNumLEDs);
    Show();
  }

  void DisplayColorChannel(int aChannel) {
    ggColor::cHSV vHSV = mHSV[0];
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

  void DisplayNormal() {
    UpdateOutput();
  }

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggLEDRing", aName);
    vDebug.PrintF("mOn=%d\n", mOn);
    vDebug.PrintF("mHSV[0]=%d/%d/%d\n", mHSV[0].Get().mH, mHSV[0].Get().mS, mHSV[0].Get().mV);
    vDebug.PrintF("mHSV[1]=%d/%d/%d\n", mHSV[1].Get().mH, mHSV[1].Get().mS, mHSV[1].Get().mV);
    vDebug.PrintF("mHSV[2]=%d/%d/%d\n", mHSV[2].Get().mH, mHSV[2].Get().mS, mHSV[2].Get().mV);
    vDebug.PrintF("mHSV[3]=%d/%d/%d\n", mHSV[3].Get().mH, mHSV[3].Get().mS, mHSV[3].Get().mV);
    vDebug.PrintF("mLEDsA[0..%d]=\n", mLEDsA.numPixels() - 1);
    for (int vIndex = 0; vIndex < mLEDsA.numPixels(); vIndex++) {
      ggColor::cRGB vRGB(mLEDsA.getPixelColor(vIndex));
      vDebug.GetStream().printf("%d/%d/%d ", vRGB.mR, vRGB.mG, vRGB.mB);
      if ((vIndex % 5 == 4) || (vIndex + 1 == mLEDsB.numPixels())) vDebug.GetStream().printf("\n");
    }
    vDebug.PrintF("mLEDsB[0..%d]=\n", mLEDsB.numPixels() - 1);
    for (int vIndex = 0; vIndex < mLEDsB.numPixels(); vIndex++) {
      ggColor::cRGB vRGB(mLEDsB.getPixelColor(vIndex));
      vDebug.GetStream().printf("%d/%d/%d ", vRGB.mR, vRGB.mG, vRGB.mB);
      if ((vIndex % 5 == 4) || (vIndex + 1 == mLEDsB.numPixels())) vDebug.GetStream().printf("\n");
    }
  }

private:

  inline static int GetLocationIndex(ggLocations aLocations, ggLocations aLocationToCheck = ggLocations::eAll) {
    ggLocations vLocations = (ggLocations)((int)aLocations & (int)aLocationToCheck);
    switch (vLocations) {
      case ggLocations::eBackBase:  return 0;
      case ggLocations::eBackTop:   return 1;
      case ggLocations::eFrontBase: return 2;
      case ggLocations::eFrontTop:  return 3;
      default: return -1;
    }
  }

  const ggColor::cHSV& GetColor(int aLocatonIndex) const {
    if ((0 <= aLocatonIndex) && (aLocatonIndex < 4)) return mHSV[aLocatonIndex];
    else return mHSV[0];
  }

  void SetColor(const ggColor::cHSV& aHSV, int aLocatonIndex, bool& aChanged) {
    if ((0 <= aLocatonIndex) && (aLocatonIndex < 4)) {
      if (mHSV[aLocatonIndex].Get() != aHSV) {
        mHSV[aLocatonIndex] = aHSV;
        aChanged = true;
      }
    }
  }

  void Show() {
    mLEDsA.show();
    mLEDsB.show();
  }

  static void FillGradient(Adafruit_NeoPixel& aLEDs,
                           uint16_t aIndex0,
                           uint16_t aIndex1,
                           const ggColor::cRGB& aRGB0,
                           const ggColor::cRGB& aRGB1) {
    const uint16_t vRange = aIndex1 - aIndex0;
    int32_t vDeltaR = (int32_t)aRGB1.mR - (int32_t)aRGB0.mR;
    int32_t vDeltaG = (int32_t)aRGB1.mG - (int32_t)aRGB0.mG;
    int32_t vDeltaB = (int32_t)aRGB1.mB - (int32_t)aRGB0.mB;
    for (uint16_t vIndex = aIndex0; vIndex <= aIndex1; vIndex++) {
      ggColor::cRGB vRGB(aRGB0.mR + vIndex * vDeltaR / vRange,
                         aRGB0.mG + vIndex * vDeltaG / vRange,
                         aRGB0.mB + vIndex * vDeltaB / vRange);
      aLEDs.setPixelColor(vIndex, vRGB);
    }
  }

  static void FillGradient(Adafruit_NeoPixel& aLEDs,
                           uint16_t aIndex0, 
                           uint16_t aIndex1,
                           const ggColor::cHSV& aHSV0,
                           const ggColor::cHSV& aHSV1) {
    const uint16_t vRange = aIndex1 - aIndex0;
    int32_t vDeltaH = (int32_t)aHSV1.mH - (int32_t)aHSV0.mH;
    int32_t vDeltaS = (int32_t)aHSV1.mS - (int32_t)aHSV0.mS;
    int32_t vDeltaV = (int32_t)aHSV1.mV - (int32_t)aHSV0.mV;
    if (vDeltaH >=  256/2) vDeltaH -= 256;
    if (vDeltaH <= -256/2) vDeltaH += 256;
    for (uint16_t vIndex = aIndex0; vIndex <= aIndex1; vIndex++) {
      ggColor::cHSV vHSV(aHSV0.mH + vIndex * vDeltaH / vRange,
                         aHSV0.mS + vIndex * vDeltaS / vRange,
                         aHSV0.mV + vIndex * vDeltaV / vRange);
      aLEDs.setPixelColor(vIndex, ggColor::ToRGB(vHSV));
    }
  }

  void UpdateOutput() {
    if (GetOn()) {
      FillGradient(mLEDsA, 0, mLEDsA.numPixels() - 1, mHSV[0].Get(), mHSV[1].Get());
      FillGradient(mLEDsB, 0, mLEDsA.numPixels() - 1, mHSV[2].Get(), mHSV[3].Get());
    }
    else {
      mLEDsA.clear();
      mLEDsB.clear();
    }
    Show();
  }

  // basic setup
  Adafruit_NeoPixel mLEDsA;
  Adafruit_NeoPixel mLEDsB;
  bool mOn;
  
  // persistent settings
  ggValueEEPromT<ggColor::cHSV> mHSV[4];
  
};
