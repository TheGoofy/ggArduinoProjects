#pragma once

#include "ggButton.h"
#include "ggOutput.h"
#include "ggOutputPWM.h"
#include "ggSensor.h"

struct ggPeriphery {

  ggButton mKey;
  ggOutputPWM mOutput;
  ggOutput mStatusLED;
  ggSensor mSensor;

  ggPeriphery()
  : mKey(0, true), // key on gpio 0, inverted (low if pressed)
    mOutput(1, true, 1.0f), // SSR switch (optocpupler) on gpio 1, inverted, 1Hz PWM
    mStatusLED(2, true), // green status led on gpio 2, inverted
    mSensor(3), // DHT connected on gpio 3
    mBlinker(1.0, [&] () { // blink with 1 Hz
      mStatusLED.Set(!mStatusLED.Get());
    }) {
  }

  void Begin() {
    mKey.Begin();
    mOutput.Begin(0.0f); // set initial state to "off"
    mStatusLED.Begin(false); // set initial state to "off"
    mSensor.Begin();
  }

  void Run() {
    mKey.Run();
    mOutput.Run();
    mSensor.Run();
    mBlinker.Run();
  }

private:

  ggSampler mBlinker;
  
};
