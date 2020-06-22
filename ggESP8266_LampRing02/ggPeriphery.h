#pragma once

#include "ggButton.h"
#include "ggRotaryEncoder.h"
#include "ggSwitchPSU.h"
#include "ggLEDCenter.h"
#include "ggLEDRing.h"
#include "ggDisplay.h"

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
  #define M_PIN_SWITCH_PSU M_PIN_GPIO_14
  #define M_PIN_LED_A_DATA M_PIN_GPIO_02_ENBOOT
  #define M_PIN_LED_B_DATA M_PIN_GPIO_00_FLASH
  #define M_PIN_I2C_SDA    M_PIN_GPIO_04_SDA
  #define M_PIN_I2C_SCL    M_PIN_GPIO_05_SCL
#else // don't use pins that are needed for serial communication
  #define M_PIN_BUTTON     M_PIN_GPIO_15_SPI_CS // gpio 15 is not connected, button won't work
  #define M_PIN_ENCODER_A  M_PIN_GPIO_12
  #define M_PIN_ENCODER_B  M_PIN_GPIO_13
  #define M_PIN_SWITCH_PSU M_PIN_GPIO_14
  #define M_PIN_LED_A_DATA M_PIN_GPIO_02_ENBOOT
  #define M_PIN_LED_B_DATA M_PIN_GPIO_00_FLASH
  #define M_PIN_I2C_SDA    M_PIN_GPIO_04_SDA
  #define M_PIN_I2C_SCL    M_PIN_GPIO_05_SCL
#endif

// IO logic polarity
#if !M_DEBUGGING
  #define M_BUTTON_INVERT true
#else
  #define M_BUTTON_INVERT false
#endif

// LED strip settings
#if !M_TEST_ENVIRONMENT
  #define M_SWITCH_PSU_ON_DELAY 2.0f
  #define M_LED_CENTER_NUM_STRIPS 6
  #define M_LED_RING_NUM_LEDS 114
  #define M_LED_RING_A_TYPE NEO_BGR
  #define M_LED_RING_B_TYPE NEO_GBR
#else
  #define M_SWITCH_PSU_ON_DELAY 1.0f
  #define M_LED_CENTER_NUM_STRIPS 6
  #define M_LED_RING_NUM_LEDS 6
  #define M_LED_RING_A_TYPE NEO_BGR
  #define M_LED_RING_B_TYPE NEO_BGR
#endif

struct ggPeriphery {

  ggButton mButton;
  ggRotaryEncoder mEncoder;
  ggSwitchPSU mSwitchPSU;
  ggLEDCenter<M_LED_CENTER_NUM_STRIPS> mLEDCenter;
  ggLEDRing<M_LED_RING_NUM_LEDS> mLEDRing;
  ggDisplay mDisplay;

  ggPeriphery()
  : mButton(M_PIN_BUTTON, M_BUTTON_INVERT, true), // button, inverted (input signal low if pressed), enable pull-up
    mEncoder(M_PIN_ENCODER_A, M_PIN_ENCODER_B), // rotary encoder
    mSwitchPSU(M_PIN_SWITCH_PSU, false, M_SWITCH_PSU_ON_DELAY), // PSU on/off, non-inverted, on-delay
    mLEDRing(M_PIN_LED_A_DATA, M_LED_RING_A_TYPE, M_PIN_LED_B_DATA, M_LED_RING_B_TYPE),
    mLEDCenter(M_PIN_I2C_SDA, M_PIN_I2C_SCL), // uses HW I2C
    mDisplay(M_PIN_I2C_SDA, M_PIN_I2C_SCL) // uses HW I2C
  {
  }

  void Begin() {
    mButton.Begin();
    mEncoder.Begin();
    mSwitchPSU.Begin();
    mLEDRing.Begin();
    mLEDCenter.Begin();
    mDisplay.Begin();
  }

  void Run() {
    mButton.Run();
    mEncoder.Run();
    mSwitchPSU.Run();
    mLEDRing.Run();
    mLEDCenter.Run();
    mDisplay.Run();
  }

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggPeriphery", aName);
    mButton.PrintDebug("mButton");
    mSwitchPSU.PrintDebug("mSwitchPSU");
    // mEncoder.PrintDebug("mEncoder");
    // mLEDCenter.PrintDebug("mLEDCenter");
    mLEDRing.PrintDebug("mLEDRing");
    mDisplay.PrintDebug("mDisplay");
  }

private:

};
