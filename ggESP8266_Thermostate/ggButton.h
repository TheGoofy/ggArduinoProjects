#pragma once

#include <functional>

/**
 * button with software debouncing
 */
class ggButton {

public:

  typedef std::function<void()> tEventFunc;
  typedef std::function<void(bool aPressed)> tChangedFunc;

  ggButton(int aPin,
           bool aInverted = false,
           bool aEnablePullUp = false,
           int aDebounceMillis = 30)
  : mPin(aPin),
    mInverted(aInverted),
    mEnablePullUp(aEnablePullUp),
    mDebounceMillis(aDebounceMillis),
    mPressed(false),
    mMillis(0),
    mChangedFunc(nullptr),
    mPressedFunc(nullptr),
    mReleasedFunc(nullptr) {
  }

  void Begin() {
    pinMode(mPin, mEnablePullUp ? INPUT_PULLUP : INPUT);
  }

  void Run() {
    CheckInput();
  }

  bool GetPressed() const {
    return mPressed;
  }

  void OnChanged(tChangedFunc aChangedFunc) {
    mChangedFunc = aChangedFunc;
  }

  void OnPressed(tEventFunc aPressedFunc) {
    mPressedFunc = aPressedFunc;
  }

  void OnReleased(tEventFunc aReleasedFunc) {
    mReleasedFunc = aReleasedFunc;
  }

private:

  void CheckInput() {
    long vMillis = millis();
    long vMillisDelta = vMillis - mMillis;
    if (vMillisDelta > mDebounceMillis) {
      bool vPressed = digitalRead(mPin) ^ mInverted;
      if (vPressed != mPressed) {
        mPressed = vPressed;
        mMillis = vMillis;
        if (mChangedFunc != nullptr) mChangedFunc(mPressed);
        if (mPressedFunc != nullptr && mPressed) mPressedFunc();
        if (mReleasedFunc != nullptr && !mPressed) mReleasedFunc();
      }
    }
  }

  int mPin;
  bool mInverted;
  bool mEnablePullUp;
  int mDebounceMillis;
  bool mPressed;
  long mMillis;

  tChangedFunc mChangedFunc;
  tEventFunc mPressedFunc;
  tEventFunc mReleasedFunc;

};

