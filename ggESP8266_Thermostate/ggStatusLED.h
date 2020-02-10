#pragma once

#include "ggOutput.h"
#include "ggTicker.h"

class ggStatusLED {

public:

  ggStatusLED(int aPin,
              bool aInverted = false)
  : mOutput(aPin, aInverted),
    mTickerOK({1, 299}, 10),           // 0.01s on, 2.99s off
    mTickerOTA({3, 7}, 10),           // 0.03s on, 0.07s off
    mTickerWarning({5, 2, 5, 18}),     // 0.5s off, 0.2s on, 0.5s off, 1.8s on
    mTickerError({1, 2, 1, 2, 1, 23}), // 0.1s on, 0.2s off, 0.1s on, 0.2s off, 0.1s on, 2.3s off
    mOTA(false),
    mWarning(false),
    mError(false) {
    mTicker = &mTickerOK;
    mTicker->Start();
  }

  void SetOTA(bool aOTA) {
    if (mOTA != aOTA) {
      mOTA = aOTA;
      UpdateTicker();
    }
  }

  bool GetOTA() const {
    return mOTA;
  }

  void SetWarning(bool aWarning) {
    if (mWarning != aWarning) {
      mWarning = aWarning;
      UpdateTicker();
    }
  }

  bool GetWarning() const {
    return mWarning;
  }

  void SetError(bool aError) {
    if (mError != aError) {
      mError = aError;
      UpdateTicker();
    }
  }

  bool GetError() const {
    return mError;
  }

  void Begin() {
    mOutput.Begin();
    mTickerOK.OnTick([&] (int aIntervalIndex) { OnTick(aIntervalIndex); });
    mTickerOTA.OnTick([&] (int aIntervalIndex) { OnTick(aIntervalIndex); });
    mTickerWarning.OnTick([&] (int aIntervalIndex) { OnTick(aIntervalIndex); });
    mTickerError.OnTick([&] (int aIntervalIndex) { OnTick(aIntervalIndex); });
    UpdateTicker();
  }

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggStatusLED", aName);
    mOutput.PrintDebug("mOutput");
    vDebug.PrintF("mOTA = %d\n", mOTA);
    vDebug.PrintF("mWarning = %d\n", mWarning);
    vDebug.PrintF("mError = %d\n", mError);
  }

private:

  void UpdateTicker() {
    mTicker->Stop();
    if (mOTA) {
      mTicker = &mTickerOTA;
    }
    else if (mError) {
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
  ggTicker mTickerOTA;
  ggTicker mTickerWarning;
  ggTicker mTickerError;

  ggTicker* mTicker;

  bool mOTA;
  bool mWarning;
  bool mError;
  
};
