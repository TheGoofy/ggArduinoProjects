#pragma once

#include "ggButton.h"
#include "ggRotaryEncoder.h"
#include "ggLEDCenter.h"
#include "ggLEDRing.h"
#include "ggStatusLED.h"

// verbose pin description for ESP-12F pins
#define M_PIN_GPIO_00_FLASH     0 // if low at boot, ESP will be in programming mode (i.e. no normal bootup if low)
#define M_PIN_GPIO_01_TX        1 // usually used for serial communication (terminal), high at boot, debug at boot (i.e. no normal bootup if low)
#define M_PIN_GPIO_02_ENBOOT    2 // must be pulled high at boot (i.e. no bootup if low)
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
#if defined(M_PCB_VERSION_v0a)
  #define M_VERSION_HW "v0a - PCB manually wired"
  #define M_PIN_BUTTON     M_PIN_GPIO_12
  #define M_PIN_ENCODER_A  M_PIN_GPIO_04_SDA
  #define M_PIN_ENCODER_B  M_PIN_GPIO_05_SCL
  #define M_PIN_LED1_PWM   M_PIN_GPIO_14
  #define M_PIN_LED2_DATA  M_PIN_GPIO_13
  #define M_PIN_STATUS_LED M_PIN_GPIO_02_ENBOOT // onboard
  #define M_LED_RING_NUM_LEDS 24
  #define M_LED_RING_TYPE NEO_BGR
#elif defined(M_PCB_VERSION_v1a)
  #define M_VERSION_HW "v1a - PCB printed"
  #define M_PIN_BUTTON     M_PIN_GPIO_12     // SW-2
  #define M_PIN_ENCODER_A  M_PIN_GPIO_04_SDA // SW-4
  #define M_PIN_ENCODER_B  M_PIN_GPIO_05_SCL // SW-3
  #define M_PIN_LED1_PWM   M_PIN_GPIO_14
  #define M_PIN_LED2_DATA  M_PIN_GPIO_13
  #define M_PIN_STATUS_LED M_PIN_GPIO_02_ENBOOT // onboard
  #define M_LED_RING_NUM_LEDS 24
  #define M_LED_RING_TYPE NEO_BGR
#endif // M_PCB_VERSION_X

// IO logic polarity
#define M_BUTTON_INVERT true

struct ggPeriphery {

  ggButton mButton;
  ggRotaryEncoder mEncoder;
  ggLEDCenter mLEDCenter;
  ggLEDRing<M_LED_RING_NUM_LEDS> mLEDRing;
  ggStatusLED mStatusLED;

  ggPeriphery()
  : mButton(M_PIN_BUTTON, M_BUTTON_INVERT, true), // button, inverted (input signal low if pressed), enable pull-up
    mEncoder(M_PIN_ENCODER_A, M_PIN_ENCODER_B), // rotary encoder
    mLEDRing(M_PIN_LED2_DATA, M_LED_RING_TYPE),
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
  }

  void Run() {
    mButton.Run();
    mEncoder.Run();
    mLEDRing.Run();
    mLEDCenter.Run();
  }

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggPeriphery", aName);
    mButton.PrintDebug("mButton");
    // mEncoder.PrintDebug("mEncoder");
    // mLEDCenter.PrintDebug("mLEDCenter");
    mLEDRing.PrintDebug("mLEDRing");
    // mStatusLED.PrintDebug("mStatusLED");
  }

private:

};
