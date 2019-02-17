#pragma once

#include "ggButton.h"
#include "ggStatusLED.h"
#include "ggOutputPWM.h"
#include "ggSensor.h"

struct ggPeriphery {

  ggButton mKey;
  ggOutputPWM mOutput;
  ggStatusLED mStatusLED;
  ggSensor mSensor;

  ggPeriphery()
  : mKey(0, true), // key on gpio 0, inverted (low if pressed)
    mOutput(1, true, 1.0f), // SSR switch (optocpupler) on gpio 1, inverted, 1Hz PWM
    mStatusLED(2, true), // green status led on gpio 2, inverted
    mSensor(3) { // DHT connected on gpio 3
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
    mStatusLED.Run();
    mSensor.Run();
  }

};
