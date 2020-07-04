#pragma once

#include "ggValueEEPromString.h"

template <uint16_t TStringLength,
          uint16_t TNumScenesMax,
          typename TBrightness,
          uint16_t TNumBrightnesses,
          typename TColor,
          uint16_t TNumColors>

class ggDataEEPromT {

  static_assert(TNumScenesMax > 0, "max number of scenes size is too small");

public:

  typedef ggValueEEPromString<TStringLength> tString;
  typedef ggValueEEPromT<TBrightness> tBrightness;
  typedef ggValueEEPromT<TColor> tColor;

  struct cScene {
    tString mName;
    std::array<tBrightness, TNumBrightnesses> mBrightnesses;
    std::array<tColor, TNumColors> mColors;
  };

  ggDataEEPromT(const String& aName,
                uint16_t aNumScenes = 1)
  : mName(aName),
    mOn(false),
    mScenes(),
    mNumScenes(aNumScenes),
    mCurrentSceneIndex(0)
  {
  }

  inline tString& Name() {
    return mName;
  }

  inline const tString& Name() const {
    return mName;
  }

  inline ggValueEEPromT<bool>& On() {
    return mOn;
  }

  inline const ggValueEEPromT<bool>& On() const {
    return mOn;
  }

  inline cScene& CurrentScene() {
    return mScenes[mCurrentSceneIndex];
  }

  inline const cScene& CurrentScene() const {
    return mScenes[mCurrentSceneIndex];
  }

private:

  tString mName;
  ggValueEEPromT<bool> mOn;
  ggValueEEPromT<uint16_t> mNumScenes;
  ggValueEEPromT<uint16_t> mCurrentSceneIndex;

  std::array<cScene, TNumScenesMax> mScenes;

};
