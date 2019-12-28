#pragma once

#include "ggButton.h"
#include "ggRotaryEncoder.h"
#include "ggLEDCenter.h"
#include "ggLEDRing.h"
#include "ggStatusLED.h"

#define M_PIN_BUTTON 12
#define M_PIN_ENCODER_A 4
#define M_PIN_ENCODER_B 5
#define M_PIN_LED1_PWM 14
#define M_PIN_LED2_DATA 13
#define M_PIN_STATUS_LED 2 // onboard

struct ggPeriphery {

  ggValueEEPromT<bool> mOn;

  ggButton mButton;
  ggRotaryEncoder mEncoder;
  ggLEDCenter mLEDCenter;
  ggLEDRing<M_PIN_LED2_DATA, 24> mLEDRing;
  ggStatusLED mStatusLED;

  ggPeriphery()
  : mButton(M_PIN_BUTTON, true, true), // button, inverted (input signal low if pressed)
    mEncoder(M_PIN_ENCODER_A, M_PIN_ENCODER_B), // rotary encoder
    mLEDRing(),
    mLEDCenter(M_PIN_LED1_PWM),
    mStatusLED(M_PIN_STATUS_LED, true) // status led, inverted
  {
  }

  void Begin() {
    mButton.Begin();
    mEncoder.Begin();
    mLEDRing.Begin();
    mLEDCenter.Begin();
    mStatusLED.Begin();
    if (mOn.Get()) SetOn();
  }

  bool GetOn() const {
    return mOn;
  }

  void SetOn() {
    mOn = true;
    mLEDRing.SetOn(true);
    mLEDCenter.SetOn(true);
  }

  void SetOff() {
    mOn = false;
    mLEDCenter.SetOn(false);
    mLEDRing.SetOn(false);
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
  }

};
