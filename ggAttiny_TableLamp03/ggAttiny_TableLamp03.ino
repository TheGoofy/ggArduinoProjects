#include <avr/sleep.h>

#include "ggInput.h"
#include "ggOutput.h"
#include "ggSampler.h"
#include "ggButton.h"
#include "ggMorse.h"


// pin mapping attiny 84a => arduino
#define M_ATTINY_PA0_PIN  0
#define M_ATTINY_PA1_PIN  1
#define M_ATTINY_PA2_PIN  2
#define M_ATTINY_PA3_PIN  3
#define M_ATTINY_PA4_PIN  4
#define M_ATTINY_PA5_PIN  5
#define M_ATTINY_PA6_PIN  6
#define M_ATTINY_PA7_PIN  7
#define M_ATTINY_PB0_PIN 10
#define M_ATTINY_PB1_PIN  9
#define M_ATTINY_PB2_PIN  8
// #define M_ATTINY_PB3_PIN // no mapping

// periphery pin mapping
#define M_KEY_ON_PIN M_ATTINY_PA0_PIN
#define M_KEY_A_PIN  M_ATTINY_PA2_PIN
#define M_KEY_B_PIN  M_ATTINY_PA3_PIN
#define M_KEY_K_PIN  M_ATTINY_PB0_PIN
#define M_LED_A_PIN  M_ATTINY_PB2_PIN
#define M_LED_B_PIN  M_ATTINY_PA7_PIN
#define M_EN12V_PIN  M_ATTINY_PB1_PIN

// periphery objects
ggOutput mLedA(M_LED_A_PIN);
ggOutput mLedB(M_LED_B_PIN);
ggOutput mEnable12V(M_EN12V_PIN);
ggButton mButtonOn(M_KEY_ON_PIN, true/*aInverted*/, true/*aEnablePullUp*/);
ggButton mButtonA(M_KEY_A_PIN, true/*aInverted*/, true/*aEnablePullUp*/);
ggButton mButtonB(M_KEY_B_PIN, true/*aInverted*/, true/*aEnablePullUp*/);
ggButton mButtonK(M_KEY_K_PIN, true/*aInverted*/, true/*aEnablePullUp*/);

// voltage and battery constants
// Lamp A: 1086L
// Lamp B: 1128L
// Lamp C: 1091L
#define M_INTERNAL_REFERENCE_MV 1091L // measured in mV (nominal 1100mV)
#define M_BATTERY_MV_LOW   3700L // 3.70V => 25% charge
#define M_BATTERY_MV_EMPTY 3550L // 3.55V =>  5% charge


// the "display" ... ;-)
ggMorse mMorse(
  [] (long aMillis) { // aSignalFunc
    mLedA.Set(true);
    mLedB.Set(true);
    delay(aMillis);
    mLedA.Set(false);
    mLedB.Set(false);
  },
  [] (long aMillis, bool aIsSymbol) { // aSpaceFunc
    delay(aMillis);
  },
  120 // aMillisDit
);


// LED brightness and color control
namespace ggLedControl {
  unsigned int mBrightness = 64; 
  unsigned int mColor = 0;
  void PowerOn() {
    mEnable12V.Set(true);
  }
  void PowerOff() {
    mEnable12V.Set(false);
  }
  void SetOutput(int aValueA, int aValueB) {
    analogWrite(mLedA.GetPin(), aValueA);
    analogWrite(mLedB.GetPin(), aValueB);
  }  
  void SetOutput() {
    unsigned int vValueA = (mColor * mBrightness) >> 8;
    unsigned int vValueB = ((255 - mColor) * mBrightness) >> 8;
    SetOutput(vValueA, vValueB);
  }
  void SetOutputOff() {
    SetOutput(0, 0);
  }
  void BrightnessInc() {
    mBrightness += 1 + (mBrightness >> 3);
    if (mBrightness > 255) mBrightness = 255;
  }
  void BrightnessDec() {
    mBrightness -= 1 + (mBrightness >> 3);
    if (mBrightness > 255) mBrightness = 0;
    if (mBrightness < 4) mBrightness = 4;
  }
  void ColorInc() {
    mColor += 17;
    if (mColor > 255) mColor = 255;
  }
  void ColorDec() {
    mColor -= 17;
    if (mColor > 255) mColor = 0;
  }
}


// indicates, if next button-release-event should power off
bool mPowerOffArmed = false;


// helpers to set or clear a specific bit in a value
#define BIT_SET(aValue, aBit) aValue |= (1<<aBit);
#define BIT_CLR(aValue, aBit) aValue &= ~(1<<aBit);

// service routine for pin change interrupt 0 (PCINT0)
ISR (PCINT0_vect)
{
}

void PowerDownSleep()
{
  // turn off external power cunsumers
  ggLedControl::SetOutputOff();
  ggLedControl::PowerOff();
  
  // turn off internal power-consumption 
  BIT_CLR(ADCSRA, ADEN); // disable ADC (safe 200uA)

  // configure wake-up interrupt
  BIT_SET(GIMSK, PCIE0); // enable pin change interrupt for pins PCINT[7:0]
  BIT_SET(PCMSK0, PCINT0); // enable pin change interrupt for pin PCINT0
  sei(); // set Global Interrupt Enable

  // go to sleep
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  
  sleep_enable();
  sleep_cpu(); // enter sleep mode

  //
  // the programme will continue from here after the interrupt
  //

  // stay awake
  sleep_disable();

  // disarm wake-up interrupt
  BIT_CLR(GIMSK, PCIE0); // disable pin change interrupt for pins PCINT[7:0]
  BIT_CLR(PCMSK0, PCINT0); // disable pin change interrupt for pin PCINT0

  // resume internal power activities
  BIT_SET(ADCSRA, ADEN); // enable ADC

  // resume external power cunsumers
  ggLedControl::PowerOn();
  ggLedControl::SetOutput();

  // treat next button release not as "off"
  mPowerOffArmed = false;
}


uint16_t ReadVccMV()
{
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  
 
  delay(10); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // wait for conversion to complete
 
  uint16_t vAdcLow  = ADCL; // must read ADCL first - it then locks ADCH  
  uint16_t vAdcHigh = ADCH; // unlocks both
  uint32_t vAdc = (vAdcHigh << 8) | vAdcLow;
 
  uint32_t vVCC = (M_INTERNAL_REFERENCE_MV * 1024L) / vAdc; // Calculate Vcc (in mV);
                         
  return vVCC;  
}



// battery control
namespace ggBattery {
  enum tState {
    eStateEmpty,
    eStateLow,
    eStateNormal
  };
  tState mState = eStateNormal;
  void UpdateState() {
    if (millis() < 1000) return;
    static unsigned long vVccSum = 0;
    static unsigned int vVccCount = 0;
    vVccSum += ReadVccMV();
    vVccCount++;
    if (vVccCount >= 256) {
      int vVccMV = vVccSum >> 8;
      vVccSum = 0;
      vVccCount = 0;
      if (vVccMV >= M_BATTERY_MV_LOW) {
        mState = eStateNormal;
      }
      else if (vVccMV >= M_BATTERY_MV_EMPTY) {
        if (mState == eStateNormal) {
          mState = eStateLow;
        }
      }
      else {
        mState = eStateEmpty;
      }
    }
  }
  void DisplayState() {
    static ggSampler vSampler(10.0f); // 10.0 Hz
    if (vSampler.IsDue()) {
      static unsigned int vCount = 0;
      switch (mState) {
        case eStateNormal: // keep output as desired
          ggLedControl::SetOutput();
          break;
        case eStateLow: // warning flashing
          if (vCount == 0) ggLedControl::SetOutput(100, 0);
          else if (vCount == 1) ggLedControl::SetOutput(0, 100);
          else ggLedControl::SetOutput();
          break;
        case eStateEmpty: // power off
          PowerDownSleep();
          mState = eStateNormal; // avoid to immediately fall asleep again
          break;
      }
      vCount = vCount < 100 ? vCount + 1 : 0;
    }
  }
}


void setup()
{
  // configure outputs
  mLedA.Begin();
  mLedB.Begin();
  mEnable12V.Begin();

  // configure inputs
  mButtonOn.Begin();
  mButtonA.Begin();
  mButtonB.Begin();
  mButtonA.SetRepeat(1000, 100);
  mButtonB.SetRepeat(1000, 100);
  mButtonK.Begin();

  // startup with power-down
  PowerDownSleep();
}


void loop()
{
  // handle power on/off button
  if (mButtonOn.HasEvent()) {
    if (mButtonOn.EventReleased()) {
      if (mPowerOffArmed) {
        // power off and go to sleep
        PowerDownSleep();
        mPowerOffArmed = false;
      }
      else {
        // only every second event is an "off" event
        mPowerOffArmed = true;
      }
    }
    mButtonOn.EventClear();
  }

  // handle brightness or color (buttons A and B)
  static bool vChangeBrightness = true;
  if (mButtonA.HasEvent() || mButtonB.HasEvent()) {
    if (mButtonA.IsPressed() && mButtonB.IsPressed()) {
      if (!mButtonA.IsRepeating() && !mButtonB.IsRepeating()) {
        vChangeBrightness = !vChangeBrightness;
      }
    }
    else if (mButtonA.EventPressed()) {
      vChangeBrightness ? ggLedControl::BrightnessInc() : ggLedControl::ColorInc();
    }
    else if (mButtonB.EventPressed()) {
      vChangeBrightness ? ggLedControl::BrightnessDec() : ggLedControl::ColorDec();
    }
    ggLedControl::SetOutput();
    mButtonA.EventClear();
    mButtonB.EventClear();
  }

  // battery management key => display battery status
  if (mButtonK.HasEvent()) {
    if (mButtonK.EventReleased()) {
      uint16_t vVCC = ReadVccMV();
      ggLedControl::SetOutputOff();
      mMorse.Signal(" ");
      mMorse.Signal(String(vVCC).c_str());
      mMorse.Signal(" ");
      ggLedControl::SetOutput();
    }
    mButtonK.EventClear();
  }

  // read the battery voltage (low voltage protection)
  ggBattery::UpdateState();
  ggBattery::DisplayState();
}
