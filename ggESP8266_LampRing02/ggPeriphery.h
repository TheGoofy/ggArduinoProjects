#pragma once

#include "ggButton.h"
#include "ggRotaryEncoder.h"
#include "ggLEDCenter.h"
#include "ggLEDRing.h"
#include "ggDisplay.h"
#include "ggTimer.h"
#include "ggOutput.h"

// verbose pin description for ESP-12F
#define M_PIN_GPIO_00_FLASH     0 // if low at boot, ESP will be in programming mode (boot fails if low)
#define M_PIN_GPIO_01_TX        1 // usually used for serial communication (terminal), high at boot, debug at boot (boot fails if low)
#define M_PIN_GPIO_02_ENBOOT    2 // must be pulled high at boot (boot fails if low)
#define M_PIN_GPIO_03_RX        3 // usually used for serial communication (terminal), high at boot
#define M_PIN_GPIO_04_SDA       4 // fast digital IO (no side-effects), recommended for i2c
#define M_PIN_GPIO_05_SCL       5 // fast digital IO (no side-effects), recommended for i2c
#define M_PIN_GPIO_06_SPI_CLK   6 //
#define M_PIN_GPIO_07_SPI_MISO  7 //
#define M_PIN_GPIO_08_SPI_MOSI  8 //
#define M_PIN_GPIO_09_SPI_HD    9 // high at boot
#define M_PIN_GPIO_10_SPI_WP   10 // high at boot
#define M_PIN_GPIO_11_SPI_CS0  11 //
#define M_PIN_GPIO_12          12 // fast digital IO (no side-effects)
#define M_PIN_GPIO_13          13 // fast digital IO (no side-effects)
#define M_PIN_GPIO_14          14 // fast digital IO (no side-effects)
#define M_PIN_GPIO_15_SPI_CS   15 // pulled to ground (boot fails if high)
#define M_PIN_GPIO_16_XPD_DCDC 16 // high at boot, used to wake up from deep sleep
#define M_PIN_GPIO_ADC0        A0 // analog input (inefficient for digital IO)

// pins for periphery
#if !M_DEBUGGING // pins for serial communication may used for other devices
  #define M_PIN_BUTTON     M_PIN_GPIO_03_RX // button can interfere with serial communication
  #define M_PIN_ENCODER_A  M_PIN_GPIO_12
  #define M_PIN_ENCODER_B  M_PIN_GPIO_13
  #define M_PIN_ENABLE_PSU M_PIN_GPIO_14
  #define M_PIN_LED_A_DATA M_PIN_GPIO_00_FLASH
  #define M_PIN_LED_B_DATA M_PIN_GPIO_02_ENBOOT
  #define M_PIN_I2C_SDA    M_PIN_GPIO_04_SDA
  #define M_PIN_I2C_SCL    M_PIN_GPIO_05_SCL
#else // don't use pins that are needed for serial communication
  #define M_PIN_BUTTON     M_PIN_GPIO_15_SPI_CS // gpio 15 is not connected, button won't work
  #define M_PIN_ENCODER_A  M_PIN_GPIO_12
  #define M_PIN_ENCODER_B  M_PIN_GPIO_13
  #define M_PIN_ENABLE_PSU M_PIN_GPIO_14
  #define M_PIN_LED_A_DATA M_PIN_GPIO_00_FLASH
  #define M_PIN_LED_B_DATA M_PIN_GPIO_02_ENBOOT
  #define M_PIN_I2C_SDA    M_PIN_GPIO_04_SDA
  #define M_PIN_I2C_SCL    M_PIN_GPIO_05_SCL
#endif

// IO logic polarity
#ifndef M_PRESERVE_SERIAL_PINS_FOR_DEBUGGING
  #define M_BUTTON_INVERT true
#else
  #define M_BUTTON_INVERT false
#endif

struct ggPeriphery {

  ggValueEEPromT<bool> mOn;

  ggButton mButton;
  ggRotaryEncoder mEncoder;
  ggOutput mEnablePSU;
  ggLEDCenter<6> mLEDCenter;
  ggLEDRing<24*4> mLEDRing;
  ggDisplay mDisplay;

  ggPeriphery()
  : mButton(M_PIN_BUTTON, M_BUTTON_INVERT, true), // button, inverted (input signal low if pressed), enable pull-up
    mEncoder(M_PIN_ENCODER_A, M_PIN_ENCODER_B), // rotary encoder
    mEnablePSU(M_PIN_ENABLE_PSU, false), // PSU on/off, non-inverted
    mLEDRing(M_PIN_LED_A_DATA, M_PIN_LED_B_DATA),
    mLEDCenter(M_PIN_I2C_SDA, M_PIN_I2C_SCL), // uses HW I2C
    mDisplay(M_PIN_I2C_SDA, M_PIN_I2C_SCL), // uses HW I2C
    mTimerPSU(2.0f) // 1 sec (for delay after power-on)
  {
    mTimerPSU.OnTimeOut([&] () {
      mLEDRing.DisplayNormal();
    });
  }

  void Begin() {
    mButton.Begin();
    mEncoder.Begin();
    mEnablePSU.Begin();
    mLEDRing.Begin();
    mLEDCenter.Begin();
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
    mEnablePSU.Set(true);
    mTimerPSU.Reset();
    mLEDRing.SetOn(true);
    mLEDCenter.SetOn(true);
    mDisplay.SetOn(true);    
  }

  void SetOff() {
    mOn = false;
    mEnablePSU.Set(false);
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
    mTimerPSU.Run();
    mButton.Run();
    mEncoder.Run();
    mDisplay.Run();
  }

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggPeriphery", aName);
    // mOn.PrintDebug("mOn");
    mButton.PrintDebug("mButton");
    mEnablePSU.PrintDebug("mEnablePSU");
    // mEncoder.PrintDebug("mEncoder");
    // mLEDCenter.PrintDebug("mLEDCenter");
    // mLEDRing.PrintDebug("mLEDRing");
    mDisplay.PrintDebug("mDisplay");
  }

private:

  ggTimer mTimerPSU;

};
