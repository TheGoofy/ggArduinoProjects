#pragma once

/**
 * @brief A class for handling a digital output pin with optional inversion.
 *
 * This class provides methods to configure a digital pin as an output and to
 * read or write its state. It supports optional inversion of the output logic.
 */
template<uint8_t TPin,
         bool TInverted = false>
class ggOutputT
{
public:

  ggOutputT() {
  }

  void Begin() {
    pinMode(TPin, OUTPUT);
  }

  inline bool Read() const {
    return digitalRead(TPin) ^ TInverted;
  }

  inline void Write(bool aValue) {
    digitalWrite(TPin, aValue ^ TInverted);
  }

  inline uint8_t GetPin() const {
    return TPin;
  }

  inline bool GetInverted() const {
    return TInverted;
  }

  inline void SetPinModeInput() {
    pinMode(TPin, INPUT);
  }

  inline void SetPinModeOutput() {
    pinMode(TPin, OUTPUT);
  }

};