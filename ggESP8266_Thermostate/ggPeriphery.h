#pragma once

#include "ggButton.h"
#include "ggStatusLED.h"
#include "ggOutputPWM.h"
#include "ggSensor.h"
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

#if defined(M_PCB_VERSION_V1)
  #define M_PIN_KEY 0
  #define M_PIN_SSR 1
  #define M_PIN_LED 2
  #define M_PIN_DHT 3
#elif defined (M_PCB_VERSION_V2)
  #define M_PIN_KEY 0
  #define M_PIN_SSR 3
  #define M_PIN_LED 2
  #define M_PIN_DHT 1
#elif defined (M_PCB_VERSION_V3)
  #define M_PIN_KEY 0
  #define M_PIN_SSR 3
  #define M_PIN_LED 1
  #define M_PIN_DHT 2
#elif defined (M_PCB_VERSION_V4)
  #define M_PIN_KEY    M_PIN_GPIO_00_FLASH // communication key (on / off / reset settings)
  #define M_PIN_SSR    M_PIN_GPIO_03_RX // SSR output
  #define M_PIN_LED    M_PIN_GPIO_01_TX // status LED
  #define M_PIN_SDA    M_PIN_GPIO_04_SDA // sensor I2C data
  #define M_PIN_SCL    M_PIN_GPIO_05_SCL // sensor I2C clock
#elif defined (M_PCB_VERSION_V5)
  #define M_PIN_KEY    M_PIN_GPIO_00_FLASH // communication key (on / off / reset settings)
  #define M_PIN_SSR    M_PIN_GPIO_14 // SSR output
  #define M_PIN_SSR_SW M_PIN_GPIO_12 // sensor SSR manual key (switch on)
  #define M_PIN_LED    M_PIN_GPIO_13 // status LED
  #define M_PIN_SDA    M_PIN_GPIO_04_SDA // sensor I2C data
  #define M_PIN_SCL    M_PIN_GPIO_05_SCL // sensor I2C clock
#endif // M_PCB_VERSION_X

struct ggPeriphery {

  ggButton mKey;
  ggOutputPWM mOutputPWM;
  ggStatusLED mStatusLED;
  ggSensor mSensor;
  ggDisplay mDisplay;

  ggPeriphery()
  : mKey(M_PIN_KEY, true), // key, inverted (low if pressed)
    mOutputPWM(M_PIN_SSR, true, 1.0f), // SSR switch (optocpupler), inverted, 1Hz PWM (200 half-waves per sec => 200 pwm steps)
    mStatusLED(M_PIN_LED, true), // green status led, inverted
    mSensor(M_PIN_SDA, M_PIN_SCL), // temperature sensor
    mDisplay() { // uses HW I2C on pins 4 and 5
  }

  void Begin() {
    mKey.Begin();
    mOutputPWM.Begin(0.0f); // set initial state to "off"
    mStatusLED.Begin();
    mSensor.Begin();
    mDisplay.Begin();
  }

  void Run() {
    mKey.Run();
    mSensor.Run();
    mDisplay.Run();
  }

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggPeriphery", aName);
    mKey.PrintDebug("mKey");
    mOutputPWM.PrintDebug("mOutputPWM");
    mStatusLED.PrintDebug("mStatusLED");
    mSensor.PrintDebug("mSensor");
    mDisplay.PrintDebug("mDisplay");
  }

};
