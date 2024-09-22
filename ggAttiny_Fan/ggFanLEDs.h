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
    static uint8_t vAnimation = 0;
    if (vAnimation / 5 % 2 == 0) {
      static int8_t vIndexY = 0;
      static int8_t vIncY = 1;
      if (vIndexY < 0) {
        vIndexY = 0;
        vIncY = 1;
        vAnimation++;
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
    if (vAnimation / 5 % 2 == 1) {
      static int8_t vIndexX = 0;
      static int8_t vIncX = 1;
      if (vIndexX < 0) {
        vIndexX = 0;
        vIncX = 1;
        vAnimation++;
      }
      else if (vIndexX >= GetSizeX_H()) {
        vIndexX = GetSizeX_H() - 1;
        vIncX = -1;
      }
      if (vIndexX > 0) {
        for (uint8_t vIndexY = 0; vIndexY < GetSizeY_H(); vIndexY++) {
          SetPixelXY_H(vIndexX - 1, vIndexY, GetColor(50, 0, 0));
        }
      }
      for (uint8_t vIndexY = 0; vIndexY < GetSizeY_H(); vIndexY++) {
        SetPixelXY_H(vIndexX, vIndexY, GetColor(255, 100, 0));
      }
      if (vIndexX < GetSizeX_H() - 1) {
        for (uint8_t vIndexY = 0; vIndexY < GetSizeY_H(); vIndexY++) {
          SetPixelXY_H(vIndexX + 1, vIndexY, GetColor(0, 0, 8));
        }
      }
      Show();
      vIndexX = vIndexX + vIncX;
    }
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
    static constexpr uint8_t vBlockSize = TNumLEDs / 2;
    const uint8_t vFanIndex = aIndexY / vBlockSize;
    const uint8_t vLedIndex = aIndexY % vBlockSize;
    return GetIndex(vFanIndex, aIndexX == 0 ? vLedIndex : TNumLEDs - vLedIndex - 1);
  }

  static constexpr uint8_t GetSizeX_H() {
    return TNumFans * TNumLEDs / GetSizeY_H();
  }

  static constexpr uint8_t GetSizeY_H() {
    return 2 * TNumFans;
  }

  inline uint8_t GetIndexXY_H(uint8_t aIndexX, uint8_t aIndexY) const {
    static constexpr uint8_t vBlockSize = TNumLEDs / 2;
    const uint8_t vFanIndex = aIndexY / 2;
    const uint8_t vLedIndex = (aIndexX + vBlockSize / 2) % TNumLEDs;
    return GetIndex(vFanIndex, aIndexY % 2 == 0 ? (TNumLEDs + vBlockSize - vLedIndex - 1) % TNumLEDs : vLedIndex);
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

  inline void SetPixelXY_H(uint8_t aIndexX, uint8_t aIndexY, uint32_t aColor) {
    mLEDs.setPixelColor(GetIndexXY_H(aIndexX, aIndexY), aColor);
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