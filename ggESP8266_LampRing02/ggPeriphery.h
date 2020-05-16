#pragma once

#include "ggButton.h"
#include "ggRotaryEncoder.h"
#include "ggLEDCenter.h"
#include "ggLEDRing.h"
#include "ggStatusLED.h"
#include "ggDisplay.h"

// general pin description for ESP-12F
#define M_PIN_GPIO_00_FLASH  0 // if low at boot, ESP will be in programming mode (boot fails if low)
#define M_PIN_GPIO_01_TX     1 // usually used for serial communication (terminal), debug at boot (boot fails if low)
#define M_PIN_GPIO_02_ENBOOT 2 // must be pulled high at boot (boot fails if low)
#define M_PIN_GPIO_03_RX     3 // usually used for serial communication (terminal)
#define M_PIN_GPIO_04_SDA    4 // fast digital IO (no side-effects)
#define M_PIN_GPIO_05_SCL    5 // fast digital IO (no side-effects)
#define M_PIN_GPIO_12       12 // fast digital IO (no side-effects)
#define M_PIN_GPIO_13       13 // fast digital IO (no side-effects)
#define M_PIN_GPIO_14       14 // fast digital IO (no side-effects)
#define M_PIN_GPIO_ADC0     A0 // analog input (inefficient for digital IO)

// pins for periphery
#define M_PIN_STATUS_LED M_PIN_GPIO_01_TX
#define M_PIN_BUTTON     M_PIN_GPIO_03_RX
#define M_PIN_ENCODER_A  M_PIN_GPIO_12
#define M_PIN_ENCODER_B  M_PIN_GPIO_13
#define M_PIN_LED_A_DATA M_PIN_GPIO_00_FLASH
#define M_PIN_LED_B_DATA M_PIN_GPIO_02_BOOT
#define M_PIN_I2C_SDA    M_PIN_GPIO_04_SDA
#define M_PIN_I2C_SCL    M_PIN_GPIO_05_SCL

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
    mLEDCenter(), // uses HW I2C
    mStatusLED(M_PIN_STATUS_LED, true), // status led, inverted
    mDisplay() // uses HW I2C
  {
  }

  void Begin() {
    Wire.begin(M_PIN_I2C_SDA, M_PIN_I2C_SCL);
    mButton.Begin();
    mEncoder.Begin();
    mLEDRing.Begin();
    mLEDCenter.Begin();
    mStatusLED.Begin();
    mDisplay.Begin();
    if (mOn.Get()) SetOn();
  }

  void ResetSettings()
  {
    SetOff();
    mLEDRing.ResetSettings();
    mLEDCenter.ResetSettings();
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

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggPeriphery", aName);
    // mOn.PrintDebug("mOn");
    mButton.PrintDebug("mButton");
    // mEncoder.PrintDebug("mEncoder");
    // mLEDCenter.PrintDebug("mLEDCenter");
    // mLEDRing.PrintDebug("mLEDRing");
    mStatusLED.PrintDebug("mStatusLED");
    mDisplay.PrintDebug("mDisplay");
  }

};
