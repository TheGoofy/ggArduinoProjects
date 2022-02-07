#include "ggSampler.h"
#include "ggInput.h"
#include "ggOutput.h"
#include "ggButtonSimple.h"


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
#define M_PIN_CH_OUT_8 10
#define M_PIN_CH_OUT_9 11

// functional pin assignments
#define M_PIN_STATUS_LED 13
#define M_PIN_KEY        12
#define M_PIN_RELAY_12V  M_PIN_CH_OUT_8
#define M_PIN_RELAY_450V M_PIN_CH_OUT_9

// Status LED
ggOutput mStatusLED(M_PIN_STATUS_LED);

// User input
ggButtonSimple mKey(M_PIN_KEY, true/*aInverted*/, true/*aEnablePullUp*/);

// Relays (output)
ggOutput mRelay12V(M_PIN_RELAY_12V, true/*aInverted*/);
ggOutput mRelay450V(M_PIN_RELAY_450V, true/*aInverted*/);

// Digital inputs
ggInput mInput0(M_PIN_CH_IN_0, true/*aInverted*/, true/*aEnablePullUp*/);
ggInput mInput1(M_PIN_CH_IN_1, true/*aInverted*/, true/*aEnablePullUp*/);
ggInput mInput2(M_PIN_CH_IN_2, true/*aInverted*/, true/*aEnablePullUp*/);
ggInput mInput3(M_PIN_CH_IN_3, true/*aInverted*/, true/*aEnablePullUp*/);
ggInput mInput4(M_PIN_CH_IN_4, true/*aInverted*/, true/*aEnablePullUp*/);
ggInput mInput5(M_PIN_CH_IN_5, true/*aInverted*/, true/*aEnablePullUp*/);
ggInput mInput6(M_PIN_CH_IN_6, true/*aInverted*/, true/*aEnablePullUp*/);
ggInput mInput7(M_PIN_CH_IN_7, true/*aInverted*/, true/*aEnablePullUp*/);


// testing outputs
ggSampler mSamplerStatusLED(250000, [] (unsigned long aMicrosDelta) {
  mStatusLED.Set(!mStatusLED.Get());
});
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

  // 
  mInput0.Begin();
  mInput1.Begin();
  mInput2.Begin();
  mInput3.Begin();
  mInput4.Begin();
  mInput5.Begin();
  mInput6.Begin();
  mInput7.Begin();

  // turn off relay outputs
  mRelay12V.Set(false);
  mRelay450V.Set(false);
}


void loop()
{
  mStatusLED.Set(
    mInput0.Get() ||
    mInput1.Get() ||
    mInput2.Get() ||
    mInput3.Get() ||
    mInput4.Get() ||
    mInput5.Get() ||
    mInput6.Get() ||
    mInput7.Get());

  // mSamplerStatusLED.Run();
  mSamplerRelay12V.Run();
  mSamplerRelay450V.Run();
  // if (mKey.SwitchingOn()) Serial.println("Key press");
  // if (mKey.SwitchingOff()) Serial.println("Key release");
}
