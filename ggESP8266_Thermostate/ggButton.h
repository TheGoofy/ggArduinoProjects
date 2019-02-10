#pragma once

#include <functional>

class ggButton {

public:

  typedef std::function<void()> tEventFunc;

  ggButton(int aPin,
           bool aInv = false,
           bool aPullUp = false,
           int aDebounceMillis = 25)
  : mPin(aPin),
    mInv(aInv),
    mPullUp(aPullUp),
    mDebounceMillis(aDebounceMillis),
    mPressedFunc(nullptr),
    mReleasedFunc(nullptr) {
  }

  boid Begin() {
    pinMode(mPin, INPUT);
    digitalWrite(mPin, mPullUp);
  }

  void Run() {
  }

  void OnPressed(tEventFunc aPressedFunc) {
    mPressedFunc = aPressedFunc;
  }

  void OnReleased(tEventFunc aReleasedFunc) {
    mReleasedFunc = aReleasedFunc;
  }

private:

  int mPin;
  bool mInv;
  bool mPullUp;
  int mDebounceMillis;

  tEventFunc mPressedFunc;
  tEventFunc mReleasedFunc;

};

