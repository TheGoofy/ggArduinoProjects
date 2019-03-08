#pragma once

#include <functional>
#include "ggInput.h"

/**
 * button with software debouncing
 */
class ggButton : public ggInput {

public:

  typedef std::function<void()> tEventFunc;
  typedef std::function<void(bool aPressed)> tChangedFunc;

  ggButton(int aPin,
           bool aInverted = false,
           bool aEnablePullUp = false,
           int aDebounceMillis = 30)
  : ggInput(aPin, aInverted, aEnablePullUp),
    mDebounceMillis(aDebounceMillis),
    mPressed(false),
    mMillis(0),
    mMillisDelta(0),
    mPressedForMillis(1000),
    mPressedForDone(false),
    mChangedFunc(nullptr),
    mPressedFunc(nullptr),
    mPressedForFunc(nullptr),
    mReleasedFunc(nullptr) {
  }

  void Run() {
    CheckInput();
  }

  bool GetPressed() const {
    return mPressed;
  }

  long GetMillisDelta() const {
    return mMillisDelta;
  }

  void OnChanged(tChangedFunc aChangedFunc) {
    mChangedFunc = aChangedFunc;
  }

  void OnPressed(tEventFunc aPressedFunc) {
    mPressedFunc = aPressedFunc;
  }

  void OnPressedFor(long aMillis, tEventFunc aFunc) {
    mPressedForMillis = aMillis;
    mPressedForFunc = aFunc;
  }

  void OnReleased(tEventFunc aReleasedFunc) {
    mReleasedFunc = aReleasedFunc;
  }

private:

  void CheckInput() {
    long vMillis = millis();
    long vMillisDelta = vMillis - mMillis;
    if (vMillisDelta >= mDebounceMillis) {
      bool vPressed = Get();
      if (vPressed != mPressed) {
        mPressed = vPressed;
        mMillis = vMillis;
        mMillisDelta = vMillisDelta;
        mPressedForDone = false;
        if (mChangedFunc != nullptr) mChangedFunc(mPressed);
        if (mPressedFunc != nullptr && mPressed) mPressedFunc();
        if (mReleasedFunc != nullptr && !mPressed) mReleasedFunc();
      }
      if (vMillisDelta >= mPressedForMillis) {
        if (mPressed && !mPressedForDone) {
          if (mPressedForFunc != nullptr) mPressedForFunc();
          mPressedForDone = true;
        }
      }
    }
  }

  const int mDebounceMillis;
  
  bool mPressed;
  long mMillis;
  long mMillisDelta;
  long mPressedForMillis;
  bool mPressedForDone;

  tChangedFunc mChangedFunc;
  tEventFunc mPressedFunc;
  tEventFunc mPressedForFunc;
  tEventFunc mReleasedFunc;

};

