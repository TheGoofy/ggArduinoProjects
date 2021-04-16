#pragma once

#include <functional>

class ggSampler
{

public:

  using tSampleFunc = std::function<void(unsigned long aMicrosDelta)>;

  ggSampler(
    unsigned long aMicrosPeriod,
    tSampleFunc aSampleFunc)
  : mMicrosPeriod(aMicrosPeriod),
    mSampleFunc(aSampleFunc) {
  }

  void Run() {
    unsigned long vMicros = micros();
    unsigned long vMicrosDelta = vMicros - mMicrosLast;
    if (mMicrosPeriod <= vMicrosDelta) {
      mSampleFunc(vMicrosDelta);
      mMicrosLast = vMicros;
    }
  }

private:

  unsigned long mMicrosLast {};
  const unsigned long mMicrosPeriod;
  tSampleFunc mSampleFunc;

};
