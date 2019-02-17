#pragma once

#include "ggOutput.h"
#include "ggTicker.h"

class ggStatusLED {

public:

  ggStatusLED(int aPin,
              bool aInverted = false)
  : mOutput(aPin, aInverted),
    mTickerOK({100, 2900}),
    mTickerWarning({100, 200, 100, 2600}),
    mTickerError({100, 200, 100, 200, 100, 2300}),
    mWarning(false),
    mError(false) {
    mTicker = &mTickerOK;
  }

  void SetWarning(bool aWarning) {
    if (mWarning != aWarning) {
      mWarning = aWarning;
      UpdateTicker();
    }
  }

  void SetError(bool aError) {
    if (mError!= aError) {
      mError = aError;
      UpdateTicker();
    }
  }

  void Begin() {
    mOutput.Begin();
    mTickerOK.OnTick([&] (int aIntervalIndex) { OnTick(aIntervalIndex); });
    mTickerWarning.OnTick([&] (int aIntervalIndex) { OnTick(aIntervalIndex); });
    mTickerError.OnTick([&] (int aIntervalIndex) { OnTick(aIntervalIndex); });
    UpdateTicker();
  }

  void Run() {
    mTicker->Run();
  }

private:

  void UpdateTicker() {
    if (mError) {
      mTicker = &mTickerError;
    }
    else if (mWarning) {
      mTicker = &mTickerWarning;
    }
    else {
      mTicker = &mTickerOK;
    }
    mTicker->Reset();
    mTicker->Start();
  }

  bool IsEven(int aValue) const {
    return (aValue & 1) == 0;
  }

  void OnTick(int aIntervalIndex) {
    mOutput.Set(IsEven(aIntervalIndex));
  }

  ggOutput mOutput;

  ggTicker mTickerOK;
  ggTicker mTickerWarning;
  ggTicker mTickerError;

  ggTicker* mTicker;

  bool mWarning;
  bool mError;
  
};
