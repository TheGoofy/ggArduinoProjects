#pragma once

#include "ggOutput.h"
#include "ggTimer.h"

class ggSwitchPSU
{

public:

  ggSwitchPSU(int aPin, bool aInverted,
              float aDelayOn,
              float aDelayOff)
  : mOutput(aPin, aInverted),
    mTimerOn(aDelayOn),
    mTimerOff(aDelayOff),
    mSwitchedOffFunc(nullptr)
  {
    mTimerOff.OnTimeOut([&] () {
      mOutput.Set(false);
      if (mSwitchedOffFunc != nullptr) mSwitchedOffFunc();
    });
  }

  void Begin() {
    mOutput.Begin();
  }

  void SetDelayOn(float aDelayOn) {
    mTimerOn.SetTimeOut(aDelayOn);
  }

  float GetDelayOn() const {
    mTimerOn.GetTimeOut();
  }

  void SetDelayOff(float aDelayOff) {
    mTimerOff.SetTimeOut(aDelayOff);
  }

  float GetDelayOff() const {
    mTimerOff.GetTimeOut();
  }

  void SetOn() {
    mOutput.Set(true);
    mTimerOn.Start();
    mTimerOff.Stop();
  }

  void SetOff() {
    mTimerOn.Stop();
    mTimerOff.Start();
  }

  void SetOn(bool aOn) {
    aOn ? SetOn() : SetOff();
  }

  typedef std::function<void()> tSwitchFunc;

  void OnSwitchedOn(tSwitchFunc aSwitchFunc) {
    mTimerOn.OnTimeOut(aSwitchFunc);
  }

  void OnSwitchedOff(tSwitchFunc aSwitchFunc) {
    mSwitchedOffFunc = aSwitchFunc;
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

  ggOutput mOutput;
  ggTimer mTimerOn;
  ggTimer mTimerOff;
  tSwitchFunc mSwitchedOffFunc;

};
