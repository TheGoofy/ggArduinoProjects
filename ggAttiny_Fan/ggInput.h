#pragma once

/**
 * @brief A class for handling a digital input pin with optional inversion.
 *
 * This class provides methods to configure a digital pin as an input and to
 * read its state. It supports optional inversion of the output logic.
 */
template<uint8_t TPin,
         bool TInverted = false,
         bool TEnablePullUp = true> // If true, enables the internal pull-up resistor (default is true)
class ggInputT
{
public:

  ggInputT() {
  }

  static constexpr void Begin() {
    pinMode(TPin, TEnablePullUp ? INPUT_PULLUP : INPUT);
  }

  static constexpr bool Read() {
    return digitalRead(TPin) ^ TInverted;
  }

  static constexpr uint8_t GetPin() {
    return TPin;
  }

  static constexpr bool GetInverted() {
    return TInverted;
  }

};