#include <Adafruit_NeoPixel.h> // https://github.com/adafruit/Adafruit_NeoPixel
#include "ggColor.h"
#include "ggLocations.h"
#include "ggLookupTableT.h"

template <uint16_t TNumLEDs>
class ggLEDRing {

public:

  // Front WS2812 LED-Strip RGB channel mapping
  //
  // Input   | R | G | B
  // --------+---+---+--
  // NEO_RGB | B | R | G
  // NEO_RBG | R | B | G
  // NEO_GRB | B | G | R
  // NEO_GBR | R | G | B (correct)
  // NEO_BRG | G | B | R
  // NEO_BGR | G | R | B

  ggLEDRing(int aPinA, uint16_t aTypeA,
            int aPinB, uint16_t aTypeB)
  : mLEDsA(TNumLEDs, aPinA, aTypeA + NEO_KHZ800),
    mLEDsB(TNumLEDs, aPinB, aTypeB + NEO_KHZ800)
  {
    SetTransitionTime(0.3f); // seconds
    SetColors(ggColor::cHSV::Black());
  }

  void Begin() {
    mLEDsA.begin();
    mLEDsB.begin();
    mLEDsA.clear();
    mLEDsB.clear();
    Show();
  }

  float GetTransitionTime() const {
    return mHSV.front().GetSeconds();
  }

  void SetTransitionTime(float aSeconds) {
    for (auto& vHSV : mHSV) {
      vHSV.SetSeconds(aSeconds);
    }
  }

  float GetTransitionTimeMax() const {
    float vTransitionTimeMax = 0.0f;
    for (auto& vHSV : mHSV) {
      float vTransitionTime = vHSV.GetSeconds();
      if (vTransitionTimeMax < vTransitionTime) {
        vTransitionTimeMax = vTransitionTime;
      }
    }
    return vTransitionTimeMax;
  }

  const ggColor::cHSV& GetColorHSV(ggLocations::tEnum aLocations) const {
    return GetColor(ggLocations::ToIndex(aLocations));
  }

  ggColor::cRGB GetColorRGB(ggLocations::tEnum aLocations) const {
    return ggColor::ToRGB(GetColorHSV(aLocations));
  }

  void SetColors(const ggColor::cHSV& aHSV, ggLocations::tEnum aLocations = ggLocations::eAll) {
    bool vColorChanged = false;
    SetColor(aHSV, ggLocations::ToIndex(aLocations, ggLocations::eAL), vColorChanged);
    SetColor(aHSV, ggLocations::ToIndex(aLocations, ggLocations::eAR), vColorChanged);
    SetColor(aHSV, ggLocations::ToIndex(aLocations, ggLocations::eBL), vColorChanged);
    SetColor(aHSV, ggLocations::ToIndex(aLocations, ggLocations::eBR), vColorChanged);
    if (vColorChanged) UpdateOutput();
  }

  void SetColorsBlack() {
    for (auto& vHSV : mHSV) {
      vHSV = ggColor::cHSV(vHSV.GetEnd().mH, vHSV.GetEnd().mS, 0);
    }
    UpdateOutput();
  }

  template <typename TColors>
  void SetColors(const TColors& aColors) {
    int vIndex = 0;
    bool vChanged = false;
    for (const auto& vColor : aColors) SetColor(vColor, vIndex++, vChanged);
    if (vChanged) UpdateOutput();
  }

  void DisplayColor(const ggColor::cRGB& aColor) {
    mLEDsA.fill(aColor, 0, TNumLEDs);
    mLEDsB.fill(aColor, 0, TNumLEDs);
    Show();
  }

  void DisplayColorA(const ggColor::cRGB& aColor) {
    mLEDsA.fill(aColor, 0, TNumLEDs);
    Show();
  }

  void DisplayColorB(const ggColor::cRGB& aColor) {
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
    vDebug.PrintF("mHSV[0]=%d/%d/%d\n", mHSV[0].GetEnd().mH, mHSV[0].GetEnd().mS, mHSV[0].GetEnd().mV);
    vDebug.PrintF("mHSV[1]=%d/%d/%d\n", mHSV[1].GetEnd().mH, mHSV[1].GetEnd().mS, mHSV[1].GetEnd().mV);
    vDebug.PrintF("mHSV[2]=%d/%d/%d\n", mHSV[2].GetEnd().mH, mHSV[2].GetEnd().mS, mHSV[2].GetEnd().mV);
    vDebug.PrintF("mHSV[3]=%d/%d/%d\n", mHSV[3].GetEnd().mH, mHSV[3].GetEnd().mS, mHSV[3].GetEnd().mV);
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

  void Run() {
    bool vTransitionFinished = true;
    for (const auto& vHSV : mHSV) {
      if (!vHSV.Finished()) {
        vTransitionFinished = false;
        break;
      }
    }
    if (!vTransitionFinished) {
      UpdateOutput();
    }
  }

private:

  const ggColor::cHSV& GetColor(int aLocatonIndex) const {
    if ((0 <= aLocatonIndex) && (aLocatonIndex < 4)) return mHSV[aLocatonIndex];
    else return mHSV[0];
  }

  void SetColor(const ggColor::cHSV& aHSV, int aLocatonIndex, bool& aChanged) {
    if ((0 <= aLocatonIndex) && (aLocatonIndex < 4)) {
      if (mHSV[aLocatonIndex] != aHSV) {
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
    static ggLookupTableT<int, 16> vHueLUT(0, 255, [] (int aInput) -> int {
      float vAlpha = 3.0 * M_PI * aInput / 255.0f;
      return ggRound<int>(aInput - 12.0f * sin(vAlpha));
    });
    const uint16_t vRange = aIndex1 - aIndex0;
    int32_t vDeltaH = (int32_t)aHSV1.mH - (int32_t)aHSV0.mH;
    int32_t vDeltaS = (int32_t)aHSV1.mS - (int32_t)aHSV0.mS;
    int32_t vDeltaV = (int32_t)aHSV1.mV - (int32_t)aHSV0.mV;
    if (vDeltaH >=  256/2) vDeltaH -= 256;
    if (vDeltaH <= -256/2) vDeltaH += 256;
    for (uint16_t vIndex = aIndex0; vIndex <= aIndex1; vIndex++) {
      ggColor::cHSV vHSV(vHueLUT((uint8_t)(aHSV0.mH + (vIndex * vDeltaH) / vRange)),
                         aHSV0.mS + vIndex * vDeltaS / vRange,
                         aHSV0.mV + vIndex * vDeltaV / vRange);
      aLEDs.setPixelColor(vIndex, ggColor::ToRGB(vHSV));
    }
  }

  void UpdateOutput() {
    GG_DEBUG();
    FillGradient(mLEDsA, 0, mLEDsA.numPixels() - 1, mHSV[0], mHSV[1]);
    FillGradient(mLEDsB, 0, mLEDsB.numPixels() - 1, mHSV[2], mHSV[3]);
    Show();
  }

  // basic setup
  Adafruit_NeoPixel mLEDsA;
  Adafruit_NeoPixel mLEDsB;
  std::array<ggTransitionT<ggColor::cHSV>, 4> mHSV;
  
};
