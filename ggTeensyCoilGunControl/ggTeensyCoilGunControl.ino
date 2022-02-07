#include "ggSampler.h"
#include "ggInput.h"
#include "ggOutput.h"
#include "ggButton.h"
#include "ggCoilStage.h"
#include <algorithm>


// circuit digital input channels
#define M_PIN_CH_IN_0 23
#define M_PIN_CH_IN_1 22
#define M_PIN_CH_IN_2 21
#define M_PIN_CH_IN_3 20
#define M_PIN_CH_IN_4 19
#define M_PIN_CH_IN_5 18
#define M_PIN_CH_IN_6 17
#define M_PIN_CH_IN_7 16

// circuit analog input channels
#define M_PIN_CH_AIN_0 15
#define M_PIN_CH_AIN_1 -1
#define M_PIN_CH_AIN_2 -1
#define M_PIN_CH_AIN_3 -1
#define M_PIN_CH_AIN_4 -1
#define M_PIN_CH_AIN_5 -1
#define M_PIN_CH_AIN_6 -1
#define M_PIN_CH_AIN_7 14

// circuit digital output channels
#define M_PIN_CH_OUT_0  2
#define M_PIN_CH_OUT_1  3
#define M_PIN_CH_OUT_2  4
#define M_PIN_CH_OUT_3  5
#define M_PIN_CH_OUT_4  6
#define M_PIN_CH_OUT_5  7
#define M_PIN_CH_OUT_6  8
#define M_PIN_CH_OUT_7  9
#define M_PIN_CH_OUT_8 10 // relay 12V
#define M_PIN_CH_OUT_9 11 // relay 240V

// functional pin assignments
#define M_PIN_STATUS_LED 13
#define M_PIN_KEY        12
#define M_PIN_RELAY_12V  M_PIN_CH_OUT_8
#define M_PIN_RELAY_450V M_PIN_CH_OUT_9

// Status LED
ggOutput mStatusLED(M_PIN_STATUS_LED);

// User input
ggButton mKey(M_PIN_KEY, true/*aInverted*/, true/*aEnablePullUp*/);

// Relays (output)
ggOutput mRelay12V(M_PIN_RELAY_12V, true/*aInverted*/);
ggOutput mRelay450V(M_PIN_RELAY_450V, true/*aInverted*/);


typedef std::array<ggCoilStage, 8> ggCoilStages;


// coil stages
ggCoilStages mCoilStages{
  ggCoilStage(M_PIN_CH_IN_0, M_PIN_CH_OUT_0),
  ggCoilStage(M_PIN_CH_IN_1, M_PIN_CH_OUT_1),
  ggCoilStage(M_PIN_CH_IN_2, M_PIN_CH_OUT_2),
  ggCoilStage(M_PIN_CH_IN_3, M_PIN_CH_OUT_3),
  ggCoilStage(M_PIN_CH_IN_4, M_PIN_CH_OUT_4),
  ggCoilStage(M_PIN_CH_IN_5, M_PIN_CH_OUT_5),
  ggCoilStage(M_PIN_CH_IN_6, M_PIN_CH_OUT_6),
  ggCoilStage(M_PIN_CH_IN_7, M_PIN_CH_OUT_7)
};

// testing outputs
ggSampler mSamplerRelay12V(500000, [] (unsigned long aMicrosDelta) {
  mRelay12V.Set(!mRelay12V.Get());
});
ggSampler mSamplerRelay450V(300000, [] (unsigned long aMicrosDelta) {
  mRelay450V.Set(!mRelay450V.Get());
});

void setup()
{
  // init serial communication (for debugging)
  Serial.begin(38400);
  while (!Serial && (millis() < 1000)); // wait for max. 1 second until USB serial ready
  Serial.println("Hi Teensy Coil Gun Control!");

  // initialize pins (as input or output)
  mKey.Begin();
  mStatusLED.Begin();
  mRelay12V.Begin();
  mRelay450V.Begin();

  // setup IO-pins for all stages
  for (auto& vCoilStage : mCoilStages) vCoilStage.Begin();

  // turn off relay outputs
  mRelay12V.Set(false);
  mRelay450V.Set(false);

  // configure key callbacks
  mKey.OnPressed([](){
    Serial.println("Key pressed");
  });
  mKey.OnReleased([](){
    Serial.println("Key released");
  });
}


void loop()
{
  bool vAnyProjectile = std::any_of(mCoilStages.begin(), mCoilStages.end(), [](const ggCoilStage& aCoilStage) {
    return aCoilStage.mProjectileSen.Get();
  });
  
  mStatusLED.Set(vAnyProjectile);

  mSamplerRelay12V.Run();
  mSamplerRelay450V.Run();
  mKey.Run();
}
