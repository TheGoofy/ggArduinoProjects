#pragma once

#include "ggOutput.h"
#include "ggTimer.h"

/**
 * The power output of a PSU is not immediately available, after sending the signal to
 * the "green" enable-line. Usually it takes a second (or maybe a bit less) until
 * all output voltages (3.3V, 5V, 12V) are stable. Similarly before turning off the
 * PSU, all connected devices should be shut down properly, before the output
 * voltages are sinking to zero.
 * 
 * This class handles the enable-line, and triggers (optional) callbacks when the PSU 
 * is switched on or off. When switching on, the enable-line is turned on immediately,
 * and after a specified delay, a callback is triggered. When turning off, it waits for
 * a specified delay before the enable-line is turned off, and another callback is
 * triggered.
 */
class ggSwitchPSU
{

public:

  ggSwitchPSU(int aPin, bool aInverted,
              float aDelayOn,
              float aDelayOff)
  : mSwitchedOn(false),
    mOutput(aPin, aInverted),
    mTimerOn(aDelayOn),
    mTimerOff(aDelayOff),
    mTransitionTime(-1.0f),
    mSwitchedOnFunc(nullptr),
    mSwitchedOffFunc(nullptr)
  {
    mTimerOn.OnTimeOut([&] () {
      mSwitchedOn = true;
      if (mSwitchedOnFunc != nullptr) {
        mSwitchedOnFunc(mTransitionTime);
      }
    });
    mTimerOff.OnTimeOut([&] () {
      mSwitchedOn = false;
      mOutput.Set(false);
      if (mSwitchedOffFunc != nullptr) {
        mSwitchedOffFunc(mTransitionTime);
      }
    });
  }

  void Begin() {
    mOutput.Begin();
  }

  void SetDelayOn(float aDelayOn) {
    mTimerOn.SetTimeOut(aDelayOn);
  }

  float GetDelayOn() const {
    return mTimerOn.GetTimeOut();
  }

  void SetDelayOff(float aDelayOff) {
    mTimerOff.SetTimeOut(aDelayOff);
  }

  float GetDelayOff() const {
    return mTimerOff.GetTimeOut();
  }

  void SetOn(float aTransitionTime) {
    mTransitionTime = aTransitionTime;
    mTimerOff.Stop();
    if (!mSwitchedOn) {
      mOutput.Set(true);
      mTimerOn.Start();
    }
    else {
      if (mSwitchedOnFunc != nullptr) {
        mSwitchedOnFunc(mTransitionTime);
      }
    }
  }

  void SetOff(float aTransitionTime) {
    mTransitionTime = aTransitionTime;
    mTimerOn.Stop();
    if (mSwitchedOn) {
      mTimerOff.Start();
    }
    else {
      mOutput.Set(false);
      if (mSwitchedOffFunc != nullptr) {
        mSwitchedOffFunc(mTransitionTime);
      }
    }
  }

  void SetOn(bool aOn, float aTransitionTime) {
    aOn ? SetOn(aTransitionTime) : SetOff(aTransitionTime);
  }

  typedef std::function<void(float aTransitionTime)> tSwitchFunc;

  void OnSwitchedOn(tSwitchFunc aSwitchFunc) {
    mSwitchedOnFunc = aSwitchFunc;
  }

  void OnSwitchedOff(tSwitchFunc aSwitchFunc) {
    mSwitchedOffFunc = aSwitchFunc;
  }

  bool IsSwitchedOn() const {
    return mSwitchedOn;
  }

  void Run () {
    mTimerOn.Run();
    mTimerOff.Run();
  }

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggSwitchPSU", aName);
    mOutput.PrintDebug("mOutput");
    mTimerOn.PrintDebug("mTimerOn");
    mTimerOff.PrintDebug("mTimerOff");
    vDebug.PrintF("mSwitchedOffFunc = 0x%08X\n", std::addressof(mSwitchedOffFunc));
  }

private:

  bool mSwitchedOn;
  ggOutput mOutput;
  ggTimer mTimerOn;
  ggTimer mTimerOff;
  float mTransitionTime;
  tSwitchFunc mSwitchedOnFunc;
  tSwitchFunc mSwitchedOffFunc;

};
