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

// HW wire mapping
#define M_KEY_ON_PIN M_ATTINY_PA0_PIN
#define M_KEY_A_PIN  M_ATTINY_PA2_PIN
#define M_KEY_B_PIN  M_ATTINY_PA3_PIN
#define M_LED_A_PIN  M_ATTINY_PB2_PIN
#define M_LED_B_PIN  M_ATTINY_PA7_PIN
#define M_EN12V_PIN  M_ATTINY_PB1_PIN

// periphery
ggOutput mLedA(M_LED_A_PIN);
ggOutput mLedB(M_LED_B_PIN);
ggOutput mEnable12V(M_EN12V_PIN);
ggButton mButtonOn(M_KEY_ON_PIN, true/*aInverted*/, true/*aEnablePullUp*/);
ggButton mButtonA(M_KEY_A_PIN, true/*aInverted*/, true/*aEnablePullUp*/);
ggButton mButtonB(M_KEY_B_PIN, true/*aInverted*/, true/*aEnablePullUp*/);


void MorseOutput(ggOutput& aOutput, long aMillis)
{
  aOutput.Set(true);
  delay(aMillis);
  aOutput.Set(false);
}


ggMorse mMorse(
  [] (long aMillis) { // aSignalFunc
    MorseOutput(mLedB, aMillis);
  },
  [] (long aMillis, bool aIsSymbol) { // aSpaceFunc
    if (aIsSymbol) delay(aMillis);
    else MorseOutput(mLedA, aMillis);
  },
  150 // aMillisDit
);


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

  // power off
  mEnable12V.Set(false);
  mLedA.Set(false);
  mLedB.Set(false);

  // wait for power button signal ...
  PowerDownSleep();

  // say hi ...
  mEnable12V.Set(true);
  mMorse.Signal("Hello World!");
}


#define BIT_SET(aRegister, aBit) aRegister |= (1<<aBit);
#define BIT_CLR(aRegister, aBit) aRegister &= ~(1<<aBit);


// service routine for pin change interrupt 0 (PCINT0)
ISR (PCINT0_vect)
{
}


void PowerDownSleep()
{
  // turn off ADC power-consumption (200uA)
  BIT_CLR(ADCSRA, ADEN); // disable ADC

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

  // resume power activities
  BIT_SET(ADCSRA, ADEN); // enable ADC
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
 
  const uint32_t vInternalReferenceVoltage = 1032L; // measured in mV (nominal 1100mV)
  uint32_t vVCC = (vInternalReferenceVoltage * 1024L) / vAdc; // Calculate Vcc (in mV);
                         
  return vVCC;  
}


void loop()
{
  /*
  uint16_t vVCC = ReadVccMV();
  mMorse.Signal(String(vVCC).c_str());
  mMorse.Signal("  ");
  */
  if (mButtonOn.HasEvent()) {
    if (mButtonOn.EventReleased()) {
      //
      static bool mPowerOff = true;
      mPowerOff = !mPowerOff;
      if (mPowerOff) {
        // turn off periphery power consumption
        mEnable12V.Set(false);
        mLedA.Set(false);
        mLedB.Set(false);
        // go to sleep
        PowerDownSleep();
        // resume power activities
        mEnable12V.Set(true);
      }
    }
    mButtonOn.EventClear();
  }
  if (mButtonA.HasEvent()) {
    if (mButtonA.EventPressed()) {
      mLedA.Set(!mLedA.Get());
    }
    mButtonA.EventClear();
  }
  if (mButtonB.HasEvent()) {
    if (mButtonB.EventPressed()) {
      mLedB.Set(!mLedB.Get());
    }
    mButtonB.EventClear();
  }
}
