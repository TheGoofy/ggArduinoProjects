#pragma once

#include "ggInput.h"
#include "ggOutput.h"
#include "ggSensor.h"

struct ggPeriphery {

  ggInput mKey;
  ggOutput mSSR;
  ggOutput mStatusLED;
  ggSensor mSensor;

  ggPeriphery()
  : mKey(0, true), // key on gpio 0, inverted (low if pressed)
    mSSR(1, true), // SSR switch (optocpupler) on gpio 1, inverted
    mStatusLED(2, true), // green status led on gpio 2, inverted
    mSensor(3), // DHT connected on gpio 3
    mBlinker(1.0, [&] () { // blink with 1 Hz
      mStatusLED.Set(!mStatusLED.Get());
    }) {
  }

  void Begin() {
    mKey.Begin();
    mSSR.Begin(false); // set initial state to "off"
    mStatusLED.Begin(false); // set initial state to "off"
    mSensor.Begin();
  }

  void Run() {
    mSensor.Run();
    mBlinker.Run();
  }

private:

  ggSampler mBlinker;
  
};

