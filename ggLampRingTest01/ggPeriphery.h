#pragma once

#include "ggButton.h"
#include "ggRotaryEncoder.h"
#include "ggLEDCenter.h"
#include "ggLEDRing.h"

#define M_PIN_BUTTON 12
#define M_PIN_ENCODER_A 4
#define M_PIN_ENCODER_B 5
#define M_PIN_LED1_PWM 14
#define M_PIN_LED2_DATA 13

struct ggPeriphery {

  ggButton mButton;
  ggRotaryEncoder mEncoder;
  ggLEDCenter mLEDCenter;
  ggLEDRing<M_PIN_LED2_DATA, 24> mLEDRing;

  ggPeriphery()
  : mButton(M_PIN_BUTTON, true, true), // button, inverted (input signal low if pressed)
    mEncoder(M_PIN_ENCODER_A, M_PIN_ENCODER_B), // rotary encoder
    mLEDCenter(M_PIN_LED1_PWM),
    mLEDRing() {
  }

  void Begin() {
    mButton.Begin();
    mEncoder.Begin();
    mLEDCenter.Begin();
    mLEDRing.Begin();
  }

  void Run() {
    mButton.Run();
    mEncoder.Run();
  }

};
