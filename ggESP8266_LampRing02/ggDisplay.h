#pragma once

#include <Arduino.h>
#include <LiquidCrystalIO.h>
#include <IoAbstractionWire.h>
#include <Wire.h>
#include <vector>
#include <functional>

#include "ggStringConvertT.h"
#include "ggDebug.h"

class ggDisplay {

public:

  ggDisplay()
  : mLCD(0/*rs*/, 1/*rw*/, 2/*en*/,
         4/*d4*/, 5/*d5*/, 6/*d6*/, 7/*d7*/,
         ioFrom8574(GetAddressI2C())),
    mConnectFunc(nullptr),
    mConnected(false),
    mUpdate(false),
    mCursorY(0) {
  }

  void Begin() {
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

  void SetOn(bool aOn) {
    if (aOn) mLCD.backlight();
    else mLCD.noBacklight();
  }

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

  void PrintTitle() {
    PrintLn(mTitle);
  }

  void PrintText() {
    std::for_each(mText.begin(), mText.end(), [&] (const cText& aText) {
      PrintLn(aText);
    });
  }

  void PrintLn(const cText& aText) {
    int vCursorX = GetX(aText);
    mLCD.setCursor(vCursorX, mCursorY);
    mLCD.print(aText.mString.c_str());
    mCursorY += 1;
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
    return aText.mString.length();
  }
  
  static int GetWidth() {
    return 16;
  }

  static uint8_t GetAddressI2C() {
    return 0x27;
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
        mLCD.configureBacklightPin(3);
        mLCD.backlight();
        mLCD.begin(GetWidth(), 2);
        if (mConnectFunc != nullptr) {
          mConnectFunc(mConnected);
        }
      }
    }
  }

  void Update() {
    if (mUpdate && mConnected) {
      mLCD.clear();
      mCursorY = 0;
      PrintTitle();
      PrintText();
      mUpdate = false;
    }
  }

  mutable LiquidCrystal mLCD;
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
