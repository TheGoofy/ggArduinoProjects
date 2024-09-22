#pragma once

#include "ggAttinyPins.h"

template<uint8_t TPin,
         uint32_t TFrequencyPWM,
         bool TInverted = false>
class ggControlPWM
{
public:

  ggControlPWM() {    
  }

  static constexpr uint16_t GetDutyCycleMin() {
    return 0; // 0% duty-cycle
  }

  static constexpr uint16_t GetDutyCycleMax() {
    return (F_CPU / (GetPrescaleValue() * TFrequencyPWM)) - 1; // Calculate TOP based on desired PWM frequency and CPU frequency
  }

  static constexpr uint16_t GetDutyCycle() {
    return GetDutyCyclePrivate();
  } 

  static constexpr void SetDutyCycle(uint16_t aDutyCycle) {
    SetDutyCyclePrivate(ClampDutyCycle(aDutyCycle));
  }

  static constexpr void IncDutyCycle(uint16_t aInc) {
    const uint16_t vDutyCycle = GetDutyCyclePrivate();
    if (vDutyCycle < GetDutyCycleMax()) {
      const uint16_t vIncMax = GetDutyCycleMax() - vDutyCycle;
      SetDutyCyclePrivate(aInc < vIncMax ? vDutyCycle + aInc : GetDutyCycleMax());
    }
  }

  static constexpr void DecDutyCycle(uint16_t aDec) {
    const uint16_t vDutyCycle = GetDutyCyclePrivate();
    if (vDutyCycle > GetDutyCycleMin()) {
      const uint16_t vDecMax = vDutyCycle - GetDutyCycleMin();
      SetDutyCyclePrivate(aDec < vDecMax ? vDutyCycle - aDec : GetDutyCycleMin());
    }
  }

  static constexpr void Begin() {
    // check if pin compatible with the desired timer below ...
    static_assert(TPin == M_ATTINY_PA6_PIN, "Pin not compatible with desired timer");

    // Set PA6 (OC1A) or PA5 (OC1B) as an output
    pinMode(TPin, OUTPUT);

    // Configure Timer1 for Fast PWM with ICR1 as TOP
    const uint16_t vPrescaleValue = GetPrescaleValue();
    TCCR1A = GetTCCR1AConfig(); // Set PWM mode (inverted or non-inverted)
    TCCR1B = _BV(WGM13) | _BV(WGM12) | GetPrescaleClockSelectBits(vPrescaleValue);

    // Set the TOP value to achieve desired PWM frequency
    ICR1 = GetDutyCycleMax();

    // Set the PWM duty cycle (0 - ICR1)
    OCR1A = (GetDutyCycleMin() + GetDutyCycleMax()) / 2; // Start at 50% duty cycle
  }

  inline void Run() {
  }

private:

  template <typename TValue>
  static constexpr TValue Clamp(TValue aValue, TValue aMin, TValue aMax) {
    return (aValue < aMin) ? aMin : (aValue > aMax) ? aMax : aValue;
  }

  static constexpr uint16_t ClampDutyCycle(uint16_t aDutyCycle) {
    return Clamp(aDutyCycle, GetDutyCycleMin(), GetDutyCycleMax());
  }

  static constexpr uint16_t GetDutyCyclePrivate() {
    return OCR1A;
  }

  static constexpr void SetDutyCyclePrivate(uint16_t aDutyCycle) {
    OCR1A = aDutyCycle;
  }

  static constexpr uint32_t GetTopValue() {
    return F_CPU / TFrequencyPWM;
  }

  static constexpr uint16_t GetPrescaleValue() {
    return (GetTopValue() <= 65536) ? 1 :
           (GetTopValue() / 8 <= 65536) ? 8 :
           (GetTopValue() / 64 <= 65536) ? 64 :
           (GetTopValue() / 256 <= 65536) ? 256 :
           1024;
  }

  static constexpr uint8_t GetPrescaleClockSelectBits(uint16_t aPrescaleValue) {
    return (aPrescaleValue == 1) ? _BV(CS10) :
           (aPrescaleValue == 8) ? _BV(CS11) :
           (aPrescaleValue == 64) ? _BV(CS10) | _BV(CS11) :
           (aPrescaleValue == 256) ? _BV(CS12) :
           (aPrescaleValue == 1024) ? _BV(CS10) | _BV(CS12) :
           0; // No clock source (Timer/Counter stopped)
  }

  static constexpr uint8_t GetTCCR1AConfig() {
    // Configure TCCR1A based on whether PWM is inverted or not
    return TInverted ? (_BV(COM1A1) | _BV(COM1A0) | _BV(WGM11)) // Inverting mode
                     : (_BV(COM1A1) | _BV(WGM11));              // Non-inverting mode
  }

};
