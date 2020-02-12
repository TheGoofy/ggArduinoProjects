#pragma once

#include "ggButton.h"
#include "ggStatusLED.h"
#include "ggOutputPWM.h"
#include "ggSensor.h"
#include "ggDisplay.h"

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
  #define M_PIN_KEY 0 // communication key (on / off / reset settings)
  #define M_PIN_SSR 3 // SSR output
  #define M_PIN_LED 1 // status LED
  #define M_PIN_SDA 4 // sensor I2C data
  #define M_PIN_SCL 5 // sensor I2C clock
#elif defined (M_PCB_VERSION_V5)
  #define M_PIN_KEY 0 // communication key (on / off / reset settings)
  #define M_PIN_SSR 14 // SSR output
  #define M_PIN_SSR_SW 12 // sensor SSR manual key (switch on)
  #define M_PIN_LED 13 // status LED
  #define M_PIN_SDA 4 // sensor I2C data
  #define M_PIN_SCL 5 // sensor I2C clock
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
    mOutputPWM.Run();
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
