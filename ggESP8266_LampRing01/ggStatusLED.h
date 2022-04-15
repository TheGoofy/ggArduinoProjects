#pragma once

#include "ggOutput.h"
#include "ggTicker.h"

class ggStatusLED {

public:

  ggStatusLED(int aPin,
              bool aInverted = false)
  : mOutput(aPin, aInverted),
    mTickerOK({0, 500}, 10),           // 0.00s on, 5.00s off
    mTickerWarning({5, 2, 5, 18}),     // 0.5s off, 0.2s on, 0.5s off, 1.8s on
    mTickerError({1, 2, 1, 2, 1, 23}), // 0.1s on, 0.2s off, 0.1s on, 0.2s off, 0.1s on, 2.3s off
    mWarning(false),
    mError(false) {
    mTicker = &mTickerOK;
    mTicker->Start();
  }

  void SetWarning(bool aWarning) {
    if (mWarning != aWarning) {
      mWarning = aWarning;
      UpdateTicker();
    }
  }

  void SetError(bool aError) {
    if (mError != aError) {
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

private:

  void UpdateTicker() {
    mTicker->Stop();
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
