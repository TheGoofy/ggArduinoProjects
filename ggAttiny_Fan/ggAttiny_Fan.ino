#include "ggAttinyPins.h"
#include "ggTimer.h"
#include "ggButton.h"
#include "ggOutput.h"
#include "ggControlPWM.h"


// Define pin numbers
#define M_BUTTON_ON_PIN M_ATTINY_PA0_PIN
#define M_BUTTON_A_PIN M_ATTINY_PA1_PIN
#define M_BUTTON_B_PIN M_ATTINY_PA2_PIN
#define M_BUTTON_C_PIN M_ATTINY_PA3_PIN
#define M_FAN_PWM_PIN M_ATTINY_PA6_PIN
#define M_LED_DATA_PIN M_ATTINY_PB2_PIN
#define M_EN_5V_PIN M_ATTINY_PB0_PIN
#define M_EN_12V_PIN M_ATTINY_PB1_PIN


// periphery
ggButtonT<M_BUTTON_ON_PIN> mButtonOn;
ggButtonT<M_BUTTON_A_PIN> mButtonA;
ggButtonT<M_BUTTON_B_PIN> mButtonB;
ggButtonT<M_BUTTON_C_PIN> mButtonC;
ggOutputT<M_EN_5V_PIN, false> mEnable5V; // not inverted
ggOutputT<M_EN_12V_PIN, false> mEnable12V; // not inverted
ggControlPWM<M_FAN_PWM_PIN, 25000> mFanPWM; // 
// ggControlLED<M_LED_DATA_PIN> mFanLED;


#include "ggControlLED.h"

void AnimateFanLED()
{
  TickLEDs();
}

ggTimerT<1030000/12, AnimateFanLED> mAnimateFanLED;


// At startup the power-button is "pressed". Therefore we'll have a "release" event, which must
// not yet turn off the power. We need to wait, until the button is pressed again (power-off
// armed). Only on this (second) "release" event, we shall turn off the 5V power.
void SetupButtonOn()
{
  static bool vPowerOffArmed = false;
  mButtonOn.Begin();
  mButtonOn.OnPressed([]() {
    vPowerOffArmed = true;
  });
  mButtonOn.OnReleased([]() {    
    if (vPowerOffArmed) {
      // Setting the pins to "input" will let the external pull-up/down-resitors do its power-off-work.
      // This might work more reliable than setting the output pin HIGH or LOW (if output voltage
      // slowly drops below battery voltage, and the pin is still an output, it could power on
      // the 5V again by accident)
      mEnable12V.SetPinModeInput();
      mEnable5V.SetPinModeInput();
      // Loop infinitely until all capacitors are discharged, and Attiny shuts down
      while (true);
    }
  });
}


void SetupButtonsABC()
{
  mButtonA.Begin();
  mButtonB.Begin();
  mButtonC.Begin();

  constexpr uint16_t vSteps = 20;
  constexpr uint16_t vStepPWM = (mFanPWM.GetDutyCycleMax() - mFanPWM.GetDutyCycleMin()) / vSteps;
  mButtonA.OnPressed([]() {
    mFanPWM.DecDutyCycle(vStepPWM);
  });
  mButtonC.OnPressed([]() {
    mFanPWM.IncDutyCycle(vStepPWM);
  });
}


void setup()
{
  // on startup quickly enable 5V step-up converter:
  // - Attiny is powered by these 5V
  // - 5V supply would shutdown again, as soon as on/off button is released
  mEnable5V.Begin();
  mEnable5V.Write(true);

  // enable 12V step-up converter for fan
  mEnable12V.Begin();
  mEnable12V.Write(true);

  // power-button is special
  SetupButtonOn();
  SetupButtonsABC();

  // initialize 25kHz Fan PWM
  mFanPWM.Begin();

  // Initialize the LED strip
  SetupLED();
}


void loop()
{
  mButtonOn.Run();
  mButtonA.Run();
  mButtonB.Run();
  mButtonC.Run();
  mAnimateFanLED.Run();
}
