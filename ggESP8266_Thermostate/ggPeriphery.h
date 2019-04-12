#pragma once

#include "ggButton.h"
#include "ggStatusLED.h"
#include "ggOutputPWM.h"
#include "ggSensor.h"

// #define M_PCB_VERSION_V1
// #define M_PCB_VERSION_V2
// #define M_PCB_VERSION_V3
#define M_PCB_VERSION_V4

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
  #define M_PIN_KEY 0
  #define M_PIN_SSR 3
  #define M_PIN_LED 1
  #define M_PIN_SDA 4
  #define M_PIN_SCL 5
#endif // M_PCB_VERSION_X

struct ggPeriphery {

  ggButton mKey;
  ggOutputPWM mOutput;
  ggStatusLED mStatusLED;
  ggSensor mSensor;

  ggPeriphery()
  : mKey(M_PIN_KEY, true), // key, inverted (low if pressed)
    mOutput(M_PIN_SSR, true, 0.2f), // SSR switch (optocpupler), inverted, 5Hz PWM
    mStatusLED(M_PIN_LED, true), // green status led, inverted
    mSensor(M_PIN_SDA, M_PIN_SCL) { // temperature sensor
  }

  void Begin() {
    mKey.Begin();
    mOutput.Begin(0.0f); // set initial state to "off"
    mStatusLED.Begin();
    mSensor.Begin();
  }

  void Run() {
    mKey.Run();
    mOutput.Run();
    mSensor.Run();
  }

};
