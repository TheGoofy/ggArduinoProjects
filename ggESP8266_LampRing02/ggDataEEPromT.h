#pragma once

#include "ggValueEEPromString.h"

template <uint16_t TStringLength,
          uint16_t TNumScenes,
          typename TBrightness,
          uint16_t TNumBrightnesses,
          typename TColor,
          uint16_t TNumColors>

class ggDataEEPromT {

public:

  typedef ggValueEEPromString<TStringLength> tString;
  typedef ggValueEEPromT<TBrightness> tBrightness;
  typedef ggValueEEPromT<TColor> tColor;

  ggDataEEPromT(const String& aName,
                uint16_t aNumScenes = 1)
  : mName(aName),
    mOn(false),
    mNumScenes(aNumScenes),
    mScenes(),
    mCurrentScene(mScenes[0])
  {
  }

  struct cScene {
    tString mName;
    std::array<tBrightness, TNumBrightnesses>  mBrightnesses;
    std::array<tColor, TNumColors> mColors;
  };

  void SetSceneName(uint16_t aIndex,
                    const String& aName) {
    if (aIndex < TNumScenes && aIndex < mNumScenes) {
      cScene& vScene = mScenes[aIndex];
      vScene.mName = aName;
    }
  }

  void SetSceneBrightnesses(uint16_t aIndex,
                            const TBrightness& aBrightness) {
    if (aIndex < TNumScenes && aIndex < mNumScenes) {
      ggValueEEProm::cLazyWriter vLazyWriter;
      cScene& vScene = mScenes[aIndex];
      std::for_each(vScene.mBrightnesses.begin(), vScene.mBrightnesses.end(), [&] (tBrightness& aSceneBrightness) {
        aSceneBrightness = aBrightness;
      });
    }
  }

  void SetSceneColors(uint16_t aIndex,
                      const TColor& aColor) {
    if (aIndex < TNumScenes && aIndex < mNumScenes) {
      ggValueEEProm::cLazyWriter vLazyWriter;
      cScene& vScene = mScenes[aIndex];
      std::for_each(vScene.mColors.begin(), vScene.mColors.end(), [&] (tColor& aSceneColor) {
        aSceneColor= aColor;
      });
    }
  }

  void SetScene(uint16_t aIndex,
                const String& aName,
                const TBrightness& aBrightness,
                const TColor& aColor) {
    ggValueEEProm::cLazyWriter vLazyWriter;
    SetSceneName(aIndex, aName);
    SetSceneBrightnesses(aIndex, aBrightness);
    SetSceneColors(aIndex, aColor);
  }

  tString mName;
  ggValueEEPromT<bool> mOn;

  ggValueEEPromT<uint16_t> mNumScenes;
  std::array<cScene, TNumScenes> mScenes;

  cScene& mCurrentScene;

};
