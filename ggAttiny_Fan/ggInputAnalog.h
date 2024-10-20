#include "Arduino.h"
#pragma once

template<uint8_t TPin,
         uint8_t TReferenceSelection = INTERNAL, // DEFAULT (VCC), EXTERNAL (PA0), INTERNAL (1100mV)
         uint16_t TReferenceMilliVolts = 1100,
         int16_t TOffsetMilliVolts = 0>
class ggInputAnalogT
{
public:

  ggInputAnalogT() {
  }

  static constexpr void Begin() {
    pinMode(TPin, INPUT); // just input (don't enable pullup)
    analogReference(TReferenceSelection);
  }

  static constexpr int Read() {
    return analogRead(TPin);
  }

  static constexpr uint16_t ReadMV() {
    return static_cast<uint32_t>(TReferenceMilliVolts) * analogRead(TPin) / 1024L + TOffsetMilliVolts;
  }

  static constexpr uint8_t GetPin() {
    return TPin;
  }

};