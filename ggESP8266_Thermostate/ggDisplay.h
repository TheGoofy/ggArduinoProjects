#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <vector>
#include <functional>

#include "ggStringConvertT.h"
#include "ggDebug.h"

class ggDisplay {

public:

  ggDisplay()
  : mU8G2(U8G2_R0, U8X8_PIN_NONE),
    mConnectFunc(nullptr),
    mConnected(false),
    mUpdate(false),
    mCursorY(0) {
  }

  void Begin() {
    Wire.begin();
    mU8G2.begin();
    Connect();
  }

  void Run() {
    Connect();
    Update();
  }

  struct cAlign {
    enum tEnum {
      eLeft,
      eCenter,
      eRight
    };
  };

  void SetTitle(const String& aTitle, cAlign::tEnum aAlign = cAlign::eLeft) {
    mTitle.mString = aTitle;
    mTitle.mAlign = aAlign;
    mUpdate = true;
  }

  void SetText(int aLine, const String& aText, cAlign::tEnum aAlign = cAlign::eLeft) {
    mText.resize(std::max<size_t>(aLine + 1, mText.size()));
    mText[aLine].mString = aText;
    mText[aLine].mAlign = aAlign;
    mUpdate = true;
  }

  void Clear() {
    mTitle.mString = "";
    mText.clear();
    mUpdate = true;
  }

  bool IsConnected() const {
    return mConnected;
  }

  typedef std::function<void (bool aConnected)> tConnectFunc;
  void OnConnection(tConnectFunc aConnectFunc) {
    mConnectFunc = aConnectFunc;
  }

  void PrintDebug(const String& aName = "") const;

private:

  struct cText {
    String mString;
    cAlign::tEnum mAlign;
    void PrintDebug(const String& aName = "") const;
  };

  void SetTitleFont() {
    mU8G2.setFont(u8g2_font_crox3h_tf);
  }

  void SetTextFont() {
    mU8G2.setFont(u8g2_font_crox2h_tf);
  }

  void PrintTitle() {
    SetTitleFont();
    PrintLn(mTitle);
  }

  void PrintText() {
    SetTextFont();
    std::for_each(mText.begin(), mText.end(), [&] (const cText& aText) {
      PrintLn(aText);
    });
  }

  void PrintLn(const cText& aText) {
    int vCursorX = GetX(aText);
    mCursorY += mU8G2.getAscent();
    mU8G2.drawStr(vCursorX, mCursorY, aText.mString.c_str());
    mCursorY += GetSpaceY();
  }

  int GetX(const cText& aText) const {
    switch (aText.mAlign) {
      case cAlign::eLeft: return 0;
      case cAlign::eCenter: return (GetWidth() - GetTextWidth(aText)) / 2;
      case cAlign::eRight: return GetWidth() - GetTextWidth(aText);
      default: return 0;
    }
  }

  int GetTextWidth(const cText& aText) const {
    return mU8G2.getStrWidth(aText.mString.c_str());
  }

  void PrintLine() {
    mU8G2.drawLine(0, mCursorY, GetWidth() - 1, mCursorY);
    mCursorY += 1 + GetSpaceY();
  }

  int GetSpaceY() const {
    return 2;
  }

  int GetWidth() const {
    return mU8G2.getDisplayWidth();
  }

  int GetHeight() const {
    return mU8G2.getDisplayHeight();
  }

  void ClearBuffer() {
    mU8G2.clearBuffer();
  }

  void ShowBuffer() {
    mU8G2.sendBuffer();
  }

  uint8_t GetAddressI2C() const {
    return mU8G2.getU8x8()->i2c_address >> 1; // expecting "0x3C" ...
  }

  void Connect() {
    Wire.beginTransmission(GetAddressI2C());
    bool vConnected = Wire.endTransmission() == 0;
    if (mConnected) {
      mConnected = vConnected;
      if (!mConnected) {
        if (mConnectFunc != nullptr) {
          mConnectFunc(mConnected);
        }
      }
    }
    else {
      mConnected = vConnected;
      if (mConnected) {
        mU8G2.begin();
        if (mConnectFunc != nullptr) {
          mConnectFunc(mConnected);
        }
      }
    }
  }

  void Update() {
    if (mUpdate && mConnected) {
      mCursorY = 0;
      ClearBuffer();
      PrintTitle();
      PrintLine();
      PrintText();
      ShowBuffer();
      mUpdate = false;
    }
  }

  mutable U8G2_SSD1306_128X64_NONAME_F_HW_I2C mU8G2;
  mutable bool mConnected;
  tConnectFunc mConnectFunc;

  mutable int mCursorY;
  mutable bool mUpdate;
  cText mTitle;
  std::vector<cText> mText;
  
};

template <>
String ToString(const ggDisplay::cAlign::tEnum& aAlign);

template <>
ggDisplay::cAlign::tEnum FromString(const String& aString);
