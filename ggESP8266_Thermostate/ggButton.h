#pragma once

#include <functional>
#include "ggInput.h"
#include "ggDebug.h"

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
    mPressedForDone(true),
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

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggButton", aName);
    ggInput::PrintDebug();
    vDebug.PrintF("mDebounceMillis = %d\n", mDebounceMillis);
    vDebug.PrintF("mPressed = %d\n", mPressed);
    vDebug.PrintF("mMillis = %d\n", mMillis);
    vDebug.PrintF("mMillisDelta = %d\n", mMillisDelta);
    vDebug.PrintF("mPressedForMillis = %d\n", mPressedForMillis);
    vDebug.PrintF("mPressedForDone = %d\n", mPressedForDone);
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
      else if (vMillisDelta >= mPressedForMillis) {
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
