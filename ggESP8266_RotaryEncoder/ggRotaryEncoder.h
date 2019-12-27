#pragma once

#include <Arduino.h>

class ggRotaryEncoder
{
public:

  ggRotaryEncoder(int aPinA, int aPinB)
  : mValueChangedFunc(nullptr) {
    mPinA = aPinA;
    mPinB = aPinB;
    mValue = 0;
    mValueLast = 0;
  }

  void Begin() {
    pinMode(mPinA, INPUT_PULLUP);
    pinMode(mPinB, INPUT_PULLUP);
    mBitA = digitalRead(mPinA);
    mBitB = digitalRead(mPinB);
    attachInterrupt(mPinA, OnPinA, CHANGE);
    attachInterrupt(mPinB, OnPinB, CHANGE);
  }

  long GetValue() const {
    return mValue;
  }

  void SetValue(long aValue) {
    mValue = aValue;
  }

  typedef std::function<void (long aValue)> tValueChangedFunc;
  void OnValueChanged(tValueChangedFunc aValueChangedFunc) {
    mValueChangedFunc = aValueChangedFunc;
  }

  void Run() {
    if (mValueChangedFunc != nullptr) {
      long vValue = GetValue();
      if (mValueLast != vValue) {
        mValueLast = vValue;
        mValueChangedFunc(vValue);
      }
    }
  }

private:

  static ICACHE_RAM_ATTR void OnPinA() {
    if (mBitA != digitalRead(mPinA)) {
      mBitA = !mBitA;
      mValue += mBitA ^ mBitB ? 1 : -1;
    }
  }

  static ICACHE_RAM_ATTR void OnPinB() {
    if (mBitB != digitalRead(mPinB)) {
      mBitB = !mBitB;
      mValue += mBitA ^ mBitB ? -1 : 1;
    }
  }

  static int mPinA;
  static int mPinB;

  static volatile bool mBitA;
  static volatile bool mBitB;
  static volatile long mValue;
  static volatile long mValueLast;

  tValueChangedFunc mValueChangedFunc;
  
};
