#pragma once

#include "ggButton.h"
#include "ggRotaryEncoder.h"
#include "ggLEDCenter.h"
#include "ggLEDRing.h"
#include "ggStatusLED.h"
#include "ggDisplay.h"

#define M_PIN_BUTTON 3
#define M_PIN_ENCODER_A 12
#define M_PIN_ENCODER_B 13
#define M_PIN_LED_A_DATA 0
#define M_PIN_LED_B_DATA 2
#define M_PIN_STATUS_LED 1 // onboard

struct ggPeriphery {

  ggValueEEPromT<bool> mOn;

  ggButton mButton;
  ggRotaryEncoder mEncoder;
  ggLEDCenter mLEDCenter;
  ggLEDRing<64> mLEDRing;
  ggStatusLED mStatusLED;
  ggDisplay mDisplay;

  ggPeriphery()
  : mButton(M_PIN_BUTTON, true, true), // button, inverted (input signal low if pressed)
    mEncoder(M_PIN_ENCODER_A, M_PIN_ENCODER_B), // rotary encoder
    mLEDRing(M_PIN_LED_A_DATA, M_PIN_LED_B_DATA),
    mLEDCenter(), // uses HW I2C on pins 4 and 5
    mStatusLED(M_PIN_STATUS_LED, true), // status led, inverted
    mDisplay() // uses HW I2C on pins 4 and 5
  {
  }

  void Begin() {
    mButton.Begin();
    mEncoder.Begin();
    mLEDRing.Begin();
    mLEDCenter.Begin();
    mStatusLED.Begin();
    mDisplay.Begin();
    if (mOn.Get()) SetOn();
  }

  bool GetOn() const {
    return mOn;
  }

  void SetOn() {
    mOn = true;
    mLEDRing.SetOn(true);
    mLEDCenter.SetOn(true);
    mDisplay.SetOn(true);
  }

  void SetOff() {
    mOn = false;
    mLEDCenter.SetOn(false);
    mLEDRing.SetOn(false);
    mDisplay.SetOn(false);
  }

  void SetOn(bool aOn) {
    aOn ? SetOn() : SetOff();
  }

  void ToggleOnOff() {
    SetOn(!GetOn());
  }

  void Run() {
    mButton.Run();
    mEncoder.Run();
    mDisplay.Run();
  }

};
