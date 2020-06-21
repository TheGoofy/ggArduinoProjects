#pragma once

#include "ggOutput.h"
#include "ggTimer.h"

class ggSwitchPSU
{

public:

  ggSwitchPSU(int aPin, bool aInverted, float aDelayOn)
  : mOutput(aPin, aInverted),
    mTimer(aDelayOn)
  {
  }

  void Begin() {
    mOutput.Begin();
  }

  void SetOn() {
    mOutput.Set(true);
    mTimer.Start();
  }

  void SetOff() {
    mOutput.Set(false);
    mTimer.Stop();
  }

  void SetOn(bool aOn) {
    aOn ? SetOn() : SetOff();
  }

  typedef std::function<void()> tSwitchFunc;
  void OnSwitchedOn(tSwitchFunc aSwitchFunc) {
    mTimer.OnTimeOut(aSwitchFunc);
  }

  void Run () {
    mTimer.Run();
  }

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggSwitchPSU", aName);
    mOutput.PrintDebug("mOutput");
    mTimer.PrintDebug("mTimer");
  }

private:

  ggOutput mOutput;
  ggTimer mTimer;

};
