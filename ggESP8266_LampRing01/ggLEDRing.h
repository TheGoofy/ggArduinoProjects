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

  ggLEDRing(int aPin, uint16_t aType)
  : mLEDs(TNumLEDs, aPin, aType + NEO_KHZ800)
  {
    SetColors(ggColor::cHSV::Black(), ggLocations::eAll, 0.3); // transition-time
  }

  void Begin() {
    mLEDs.begin();
    mLEDs.clear();
    Show();
  }

  const ggColor::cHSV& GetColorHSV(ggLocations::tEnum aLocations) const {
    return GetColor(ggLocations::ToIndex(aLocations));
  }

  ggColor::cRGB GetColorRGB(ggLocations::tEnum aLocations) const {
    return ggColor::ToRGB(GetColorHSV(aLocations));
  }

  void SetColors(const ggColor::cHSV& aHSV, ggLocations::tEnum aLocations, float aTransitionTime) {
    bool vColorChanged = false;
    SetColor(aHSV, ggLocations::ToIndex(aLocations, ggLocations::eAL), aTransitionTime, vColorChanged);
    SetColor(aHSV, ggLocations::ToIndex(aLocations, ggLocations::eAR), aTransitionTime, vColorChanged);
    if (vColorChanged) UpdateOutput();
  }

  void SetColorsBlack(float aTransitionTime) {
    for (auto& vHSV : mHSV) {
      vHSV.Set(ggColor::cHSV(vHSV.GetEnd().mH, vHSV.GetEnd().mS, 0), aTransitionTime);
    }
    UpdateOutput();
  }

  template <typename TColors>
  void SetColorsT(const TColors& aColors, float aTransitionTime) {
    int vIndex = 0;
    bool vChanged = false;
    for (const auto& vColor : aColors) SetColor(vColor, vIndex++, aTransitionTime, vChanged);
    if (vChanged) UpdateOutput();
  }

  void DisplayColor(const ggColor::cRGB& aColor) {
    mLEDs.fill(aColor, 0, TNumLEDs);
    Show();
  }

  void DisplayColorChannel(int aChannel) {
    ggColor::cHSV vHSV = mHSV[0];
    if ((aChannel != 1) && (vHSV.mS < 128)) vHSV.mS = 128;
    if ((aChannel != 2) && (vHSV.mV < 128)) vHSV.mV = 128;
    for (int vIndex = 0; vIndex < TNumLEDs; vIndex++) {
      vHSV.mChannels[aChannel] = 256 * (vIndex + 1) / TNumLEDs - 1;
      mLEDs.setPixelColor((vIndex + 0 * TNumLEDs / 4) % TNumLEDs, ggColor::ToRGB(vHSV));
    }
    Show();
  }

  void DisplayProgress(float aProgress, const ggColor::cRGB& aColor, const ggColor::cRGB& aBackground) {
    int vProgress = TNumLEDs * aProgress + 0.5f;
    mLEDs.fill(aColor, 0, vProgress);
    mLEDs.fill(aBackground, vProgress, TNumLEDs - vProgress);
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

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggLEDRing", aName);
    vDebug.PrintF("mHSV[0]=%d/%d/%d\n", mHSV[0].GetEnd().mH, mHSV[0].GetEnd().mS, mHSV[0].GetEnd().mV);
    vDebug.PrintF("mHSV[1]=%d/%d/%d\n", mHSV[1].GetEnd().mH, mHSV[1].GetEnd().mS, mHSV[1].GetEnd().mV);
    vDebug.PrintF("mLEDs[0..%d]=\n", mLEDs.numPixels() - 1);
    for (int vIndex = 0; vIndex < mLEDs.numPixels(); vIndex++) {
      ggColor::cRGB vRGB(mLEDs.getPixelColor(vIndex));
      vDebug.GetStream().printf("%d/%d/%d ", vRGB.mR, vRGB.mG, vRGB.mB);
      if ((vIndex % 5 == 4) || (vIndex + 1 == mLEDs.numPixels())) vDebug.GetStream().printf("\n");
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
    if ((0 <= aLocatonIndex) && (aLocatonIndex < 2)) return mHSV[aLocatonIndex];
    else return mHSV[0];
  }

  void SetColor(const ggColor::cHSV& aHSV, int aLocatonIndex, float aTransitionTime, bool& aChanged) {
    if ((0 <= aLocatonIndex) && (aLocatonIndex < 2)) {
      mHSV[aLocatonIndex].SetSeconds(aTransitionTime);
      if (mHSV[aLocatonIndex] != aHSV) {
        mHSV[aLocatonIndex] = aHSV;
        aChanged = true;
      }
    }
  }

  void Show() {
    if (mFuncShowStart) mFuncShowStart();
    mLEDs.show();
    if (mFuncShowFinish) mFuncShowFinish();
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
    FillGradient(mLEDs, 0, mLEDs.numPixels() - 1, mHSV[0], mHSV[1]);
    Show();
  }

  // basic setup
  Adafruit_NeoPixel mLEDs;
  std::array<ggTransitionT<ggColor::cHSV>, 2> mHSV;
  tFunc mFuncShowStart;
  tFunc mFuncShowFinish;
  
};
