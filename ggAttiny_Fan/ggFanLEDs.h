#pragma once

#include <Adafruit_NeoPixel.h>

template<uint8_t TPin,
         uint8_t TNumFans, 
         uint8_t TNumLEDs,
         uint8_t TFirstLED,
         uint8_t TFramesPerSec = 30>
class ggFanLEDs
{
public:

  ggFanLEDs()
  : mLEDs(GetTotalNumLEDs(), TPin, NEO_GRB + NEO_KHZ800) {
  }

  void Begin() {
    mLEDs.begin();
    mLEDs.setBrightness(255);
    /*
    for (uint8_t vFanIndex = 0; vFanIndex < TNumFans; vFanIndex++) {
      SetPixel(vFanIndex, 0, GetColor(0, 255, 0));
      SetPixel(vFanIndex, TNumLEDs-1, GetColor(255, 0, 0));
    }
    */
    Show();
  }

  void Update() {
    static int8_t vIndexY = 0;
    static int8_t vIncY = 1;
    if (vIndexY < 0) {
      vIndexY = 0;
      vIncY = 1;
    }
    else if (vIndexY >= GetSizeY()) {
      vIndexY = GetSizeY() - 1;
      vIncY = -1;
    }
    if (vIndexY > 0) {
      SetPixelXY(0, vIndexY - 1, GetColor(50, 0, 4));
      SetPixelXY(1, vIndexY - 1, GetColor(50, 4, 0));
    }
    SetPixelXY(0, vIndexY, GetColor(255, 100, 0));
    SetPixelXY(1, vIndexY, GetColor(255, 100, 0));
    if (vIndexY < GetSizeY() - 1) {
      SetPixelXY(0, vIndexY + 1, GetColor(3, 0, 8));
      SetPixelXY(1, vIndexY + 1, GetColor(0, 3, 8));
    }
    Show();
    vIndexY = vIndexY + vIncY;
  }

private:

  static constexpr uint8_t GetTotalNumLEDs() {
    return TNumFans * TNumLEDs;
  }

  static constexpr uint8_t GetIndex(uint8_t aFanIndex, uint8_t aLedIndex) {
    return aFanIndex * TNumLEDs + (aLedIndex + TFirstLED) % TNumLEDs;
  }

  static constexpr uint8_t GetSizeX() {
    return 2;
  }

  static constexpr uint8_t GetSizeY() {
    return TNumFans * TNumLEDs / GetSizeX();
  }

  inline uint8_t GetIndexXY(uint8_t aIndexX, uint8_t aIndexY) const {
    static constexpr uint8_t vBlockSize = TNumLEDs / GetSizeX();
    const uint8_t vFanIndex = aIndexY / vBlockSize;
    const uint8_t vLedIndex = aIndexY % vBlockSize;
    return GetIndex(vFanIndex, aIndexX == 0 ? vLedIndex : TNumLEDs - vLedIndex - 1);
  }

  static constexpr uint32_t GetColor(uint8_t aR, uint8_t aG, uint8_t aB) {
    return Adafruit_NeoPixel::Color(aR, aG, aB);
  }

  inline void SetPixel(uint8_t aFanIndex, uint8_t aLedIndex, uint32_t aColor) {
    mLEDs.setPixelColor(GetIndex(aFanIndex, aLedIndex), aColor);
  }

  inline void SetPixelXY(uint8_t aIndexX, uint8_t aIndexY, uint32_t aColor) {
    mLEDs.setPixelColor(GetIndexXY(aIndexX, aIndexY), aColor);
  }

  inline void Show() {
    mLEDs.show();
  }

  Adafruit_NeoPixel mLEDs;

};

/*
void ShowA(uint8_t aAngle)
{
  // static uint8_t vBrightness = 0;
  // mFanLEDs.setBrightness(vBrightness++);
  auto vColorA = mFanLEDs.Color(10, 50, 0);
  auto vColorB = mFanLEDs.Color(50, 0, 20);
  for (uint8_t vIndex = 0; vIndex < mFanLEDs.numPixels(); vIndex++) {
    mFanLEDs.setPixelColor(vIndex, (vIndex + aAngle) % 12 >= 6 ? vColorA : vColorB);
  }
}


void TickLEDs()
{
  static uint8_t vAngle = 0;
  vAngle = (vAngle + 1) % 12;
  ShowA(vAngle);
  mFanLEDs.show();
}


void AnimateFanLED()
{
  TickLEDs();
}
*/