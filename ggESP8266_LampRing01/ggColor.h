#pragma once

#include <Adafruit_NeoPixel.h>


namespace ggColor {


  struct cRGB {

    union {
      struct {
        uint8_t mR;
        uint8_t mG;
        uint8_t mB;
        uint8_t mW;
      };
      uint8_t mChannels[4];
      uint32_t mData;
    };

    inline cRGB()
    : mData(0) {
    }

    inline cRGB(uint32_t aData)
    : mData(aData) {
    }

    inline cRGB(uint8_t aR, uint8_t aG, uint8_t aB)
    : mR(aR), mG(aG), mB(aB), mW(0) {
    }

    inline cRGB(const cRGB& aOther)
    : mData(aOther.mData) {
    }

    inline operator uint32_t& () {
      return mData;
    }

    inline operator const uint32_t& () const {
      return mData;
    }

    inline bool operator == (const cRGB& aOther) const {
      return mData == aOther.mData;
    }

    inline bool operator != (const cRGB& aOther) const {
      return mData != aOther.mData;
    }

    // html colors (see https://www.w3schools.com/colors/colors_names.asp)
    inline static cRGB Black() { return cRGB(0,0,0); }
    inline static cRGB Red() { return cRGB(255,0,0); }
    inline static cRGB Green() { return cRGB(0,255,0); }
    inline static cRGB Blue() { return cRGB(0,0,255); }
    inline static cRGB Yellow() { return cRGB(255,255,0); }
    inline static cRGB Cyan() { return cRGB(0,255,255); }
    inline static cRGB Magenta() { return cRGB(255,0,255); }
    inline static cRGB White() { return cRGB(255,255,255); }
    inline static cRGB DarkGray() { return cRGB(169,169,169); }
    inline static cRGB Gray() { return cRGB(128,128,128); }
    inline static cRGB LightGray() { return cRGB(211,211,211); }
    inline static cRGB Orange() { return cRGB(255,165,0); }
    inline static cRGB GreenYellow() { return cRGB(173,255,47); }
    inline static cRGB Turquoise() { return cRGB(64,224,207); }
    inline static cRGB DeepSkyBlue() { return cRGB(0,191,255); }
    inline static cRGB DeepPink() { return cRGB(255,20,147); }
    inline static cRGB DarkViolet() { return cRGB(148,0,211); }
    inline static cRGB Purple() { return cRGB(128,0,128); }

  };


  struct cHSV {

    union {
      struct {
        uint8_t mH;
        uint8_t mS;
        uint8_t mV;
      };
      uint8_t mChannels[4];
      uint32_t mData;
    };

    inline cHSV(uint8_t aH, uint8_t aS, uint8_t aV)
    : mH(aH), mS(aS), mV(aV) {
    }

    inline cHSV(const cHSV& aOther)
    : mData(aOther.mData) {
    }

    inline bool operator == (const cHSV& aOther) const {
      return mData == aOther.mData;
    }

    inline bool operator != (const cHSV& aOther) const {
      return mData != aOther.mData;
    }

  };


  inline cRGB ToRGB(const cHSV& aHSV) {
    return Adafruit_NeoPixel::ColorHSV(256*aHSV.mH, aHSV.mS, aHSV.mV);
  }


};
