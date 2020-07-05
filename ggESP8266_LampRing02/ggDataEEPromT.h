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

  ggDataEEPromT()
  : mName(),
    mOn(false),
    mScenes(),
    mNumScenes(1),
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

  inline cScene& Scene(uint16_t aIndex) {
    return aIndex < mNumScenes ? mScenes[aIndex] : mScenes[mCurrentSceneIndex];
  }

  inline const cScene& Scene(uint16_t aIndex) const {
    return aIndex < mNumScenes ? mScenes[aIndex] : mScenes[mCurrentSceneIndex];
  }

  inline uint16_t GetNumScenes() const {
    return mNumScenes;
  }

  void SetNumScenes(uint16_t aNumScenes) {
    if (aNumScenes > 1) {
      while (mNumScenes < aNumScenes) AddScene();
      while (mNumScenes > aNumScenes) RemoveScene(mNumScenes - 1);
    }
  }

  inline uint16_t GetNumScenesMax() const {
    return TNumScenesMax;
  }

  inline uint16_t GetCurrentSceneIndex() const {
    return mCurrentSceneIndex;
  }

  inline void SetCurrentSceneIndex(uint16_t aIndex) {
    if (aIndex < mNumScenes) mCurrentSceneIndex = aIndex;
  }

  bool AddScene(bool aCopyCurrent = true) {
    GG_DEBUG();
    if (mNumScenes < TNumScenesMax) {
      ggValueEEProm::cLazyWriter vLazyWriter;
      mNumScenes += 1;
      GG_DEBUG_PRINTF("mNumScenes = %d\n", mNumScenes.Get());
      if (aCopyCurrent) {
        GG_DEBUG_PRINTF("Copy current\n");
        mScenes[mNumScenes - 1] = CurrentScene();
      }
      return true;
    }
    return false;
  }

  bool RemoveScene(uint16_t aIndex) {
    if ((0 < aIndex) && (aIndex < mNumScenes)) {
      ggValueEEProm::cLazyWriter vLazyWriter;
      mNumScenes -= 1;
      if (mCurrentSceneIndex == aIndex) mCurrentSceneIndex -= 1;
      for (uint16_t vIndex = aIndex; vIndex < mNumScenes; vIndex++) {
        mScenes[vIndex] = mScenes[vIndex + 1];
      }
      return true;
    }
    return false;
  }

private:

  tString mName;
  ggValueEEPromT<bool> mOn;
  ggValueEEPromT<uint16_t> mNumScenes;
  ggValueEEPromT<uint16_t> mCurrentSceneIndex;

  std::array<cScene, TNumScenesMax> mScenes;

};
