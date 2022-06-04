#pragma once

#include "ggInput.h"

class ggButton : public ggInput
{

public:
  
  ggButton(byte aPin,
           boolean aInverted = false,
           boolean aEnablePullUp = false,
           int aDebounceMillis = 30)
  : ggInput(aPin, aInverted, aEnablePullUp),
    mDebounceMillis(aDebounceMillis),
    mPressed(false),
    mPressedBounce(false),
    mMillisBounce(0),
    mMillisDelta(0),
    mRepeatDelayMillis(0),
    mRepeatRateMillis(0),
    mRepeatMillis(0),
    mEventRepeating(false),
    mEventChanged(false),
    mEventPressed(false),
    mEventReleased(false) {
  }

  void Begin() {    
    ggInput::Begin();
    Reset();
  }

  void Reset() {
    mPressed = Get();
    mPressedBounce = mPressed;
    mMillisBounce = millis();
    mMillisDelta = 0;
  }

  void SetRepeat(long aDelayMillis,
                 long aRateMillis) {
    mRepeatDelayMillis = aDelayMillis;
    mRepeatRateMillis = aRateMillis;
  }

  bool HasEvent() const {
    const_cast<ggButton&>(*this).CheckEvent();
    return mEventChanged ||
           mEventPressed ||
           mEventReleased;
  }

  void EventClear() {
    mEventRepeating = false;
    mEventChanged = false;
    mEventPressed = false;
    mEventReleased = false;
  }

  bool EventChanged() const {
    return mEventChanged;
  }

  bool EventPressed() const {
    return mEventPressed;
  }

  bool EventReleased() const {
    return mEventReleased;
  }

  bool IsPressed() const {
    return mPressed;
  }

  bool IsRepeating() const {
    return mEventRepeating;
  }

  long GetMillisDelta() const {
    return mMillisDelta;
  }

private:

  void CheckEvent() {
    long vMillis = millis();
    bool vPressed = Get();
    if (vPressed != mPressedBounce) {
      mPressedBounce = vPressed;
      mMillisBounce = vMillis;
    }
    mMillisDelta = vMillis - mMillisBounce;
    if (mMillisDelta >= mDebounceMillis) {
      if (vPressed != mPressed) {
        mPressed = vPressed;
        mRepeatMillis = mRepeatDelayMillis;
        mEventRepeating = false;
        mEventChanged = true;
        mEventPressed = mPressed;
        mEventReleased = !mPressed;
      }
      else {
        if (mRepeatMillis > 0) {
          if (mMillisDelta >= mRepeatMillis) {
            mEventRepeating = true;
            mEventPressed = mPressed;
            mEventReleased = !mPressed;
            if (mRepeatRateMillis > 0) {
              mRepeatMillis += mRepeatRateMillis;
            }
            else {
              mRepeatMillis = 0;
            }
          }
        }
      }
    }
  }

  const int mDebounceMillis;

  bool mPressed;
  bool mPressedBounce;
  long mMillisBounce;
  long mMillisDelta;
  long mRepeatDelayMillis;
  long mRepeatRateMillis;
  long mRepeatMillis;

  bool mEventRepeating;
  bool mEventChanged;
  bool mEventPressed;
  bool mEventReleased;  
};
