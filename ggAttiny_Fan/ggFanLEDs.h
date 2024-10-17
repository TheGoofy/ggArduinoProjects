#pragma once

#include <Adafruit_NeoPixel.h>

template<uint8_t TPin, // data-pin for LEDs
         uint8_t TNumFans, // number of fans
         uint8_t TNumLEDs, // number of LEDs per fan
         uint8_t TFirstLED> // index of first LED (start-angle)
class ggFanLEDs
{
public:

  ggFanLEDs()
  : mLEDs(GetTotalNumLEDs(), TPin, NEO_GRB + NEO_KHZ800) {
  }

  void Begin() {
    mLEDs.begin();
    mLEDs.setBrightness(255);
    Show();
  }

  static constexpr uint16_t GetTotalNumLEDs() {
    return TNumFans * TNumLEDs;
  }

  static constexpr uint8_t GetIndex(uint8_t aFanIndex, uint8_t aLedIndex) {
    return aFanIndex * TNumLEDs + (aLedIndex + TFirstLED) % TNumLEDs;
  }

  static constexpr uint8_t GetSizeX_V() {
    return 2;
  }

  static constexpr uint8_t GetSizeY_V() {
    return TNumFans * TNumLEDs / GetSizeX_V();
  }

  inline uint8_t GetIndexXY_V(uint8_t aIndexX, uint8_t aIndexY) const {
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

  inline void SetPixelXY_V(uint8_t aIndexX, uint8_t aIndexY, uint32_t aColor) {
    mLEDs.setPixelColor(GetIndexXY_V(aIndexX, aIndexY), aColor);
  }

  inline void SetPixelXY_H(uint8_t aIndexX, uint8_t aIndexY, uint32_t aColor) {
    mLEDs.setPixelColor(GetIndexXY_H(aIndexX, aIndexY), aColor);
  }

  inline void Fill(uint32_t aColor) {
    mLEDs.fill(aColor);
  }

  inline void Show() {
    mLEDs.show();
  }

private:

  Adafruit_NeoPixel mLEDs;

};
