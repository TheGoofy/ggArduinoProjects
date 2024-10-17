#pragma once

#include "ggInput.h"

template<uint8_t TPin,
         uint32_t TFilterFIR = 32>
class ggFanTacho
{
public:

  ggFanTacho() {
  }

  void Begin() {
    mInput.Begin();
  }

  inline uint32_t GetMicrosPerRev() const {
    return mMicrosPerRev;
  }

  inline uint32_t GetRPM() const {
    return mMicrosPerRev != 0 ? 60*1000000L / mMicrosPerRev : 0;
  }

  void Run() {
    // compare last bit of counter with input
    if (mInput.Read() != (mCountInput & 0x01)) {
      // increment counter (when input changed)
      ++mCountInput;
      // PC-fan has 2 high- and 2 low-impulses per revolution
      if (mCountInput > 4) {
        // measure time since last rotation
        uint32_t vMicrosDelta = micros() - mMicrosLast;
        mMicrosLast += vMicrosDelta;
        // apply FIR-filter for measurement
        static_assert(TFilterFIR > 0, "TFilterFIR must be 1 or larger!");
        mMicrosPerRev = ((TFilterFIR - 1) * mMicrosPerRev + vMicrosDelta) / TFilterFIR;
        // reset rotation count
        mCountInput = 0;
      }
    }
  }

private:

  ggInputT<TPin> mInput;

  uint32_t mMicrosPerRev = 0;
  uint32_t mMicrosLast = 0;
  uint8_t mCountInput = 0;

};