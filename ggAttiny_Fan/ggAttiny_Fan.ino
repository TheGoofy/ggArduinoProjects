#include "ggAttinyPins.h"
#include "ggTimer.h"
#include "ggButton.h"
#include "ggOutput.h"
#include "ggControlPWM.h"
#include "ggTimer.h"
#include "ggFanLEDs.h"


// Define pin numbers
#define M_BUTTON_ON_PIN M_ATTINY_PA0_PIN
#define M_BUTTON_A_PIN M_ATTINY_PA1_PIN
#define M_BUTTON_B_PIN M_ATTINY_PA2_PIN
#define M_BUTTON_C_PIN M_ATTINY_PA3_PIN
#define M_FAN_PWM_PIN M_ATTINY_PA6_PIN
#define M_LED_DATA_PIN M_ATTINY_PB2_PIN
#define M_EN_5V_PIN M_ATTINY_PB0_PIN
#define M_EN_12V_PIN M_ATTINY_PB1_PIN


// Fan LEDs animation speed
#define M_FAN_LEDs_FPS (36/1)
void UpdateFanLEDs();


// periphery
ggButtonT<M_BUTTON_ON_PIN, 30, true> mButtonOn; // 30ms debounce, inverted
ggButtonT<M_BUTTON_A_PIN, 30, true> mButtonA; // 30ms debounce, inverted
ggButtonT<M_BUTTON_B_PIN, 30, true> mButtonB; // 30ms debounce, inverted
ggButtonT<M_BUTTON_C_PIN, 30, true> mButtonC; // 30ms debounce, inverted
ggOutputT<M_EN_5V_PIN, false> mEnable5V; // not inverted
ggOutputT<M_EN_12V_PIN, false> mEnable12V; // not inverted
ggControlPWM<M_FAN_PWM_PIN, 25000, true> mFanPWM; // 25kHz, inverted
ggFanLEDs<M_LED_DATA_PIN, 3, 12, 5, M_FAN_LEDs_FPS> mFanLEDs; // 3 fans, 12 LEDs per fan, led #5 ist first
ggTimerT<1000000 / M_FAN_LEDs_FPS, UpdateFanLEDs> mTimerFanLEDs;


void UpdateFanLEDs()
{
  mFanLEDs.Update();
}


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
    mFanPWM.IncDutyCycle(vStepPWM);
  });
  mButtonC.OnPressed([]() {
    mFanPWM.DecDutyCycle(vStepPWM);
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

  // initialize Fan PWM
  mFanPWM.Begin();
  mFanPWM.SetDutyCycle(mFanPWM.GetDutyCycleMax() / 4);

  // Initialize the LED strip
  mFanLEDs.Begin();
}


void loop()
{
  mButtonOn.Run();
  mButtonA.Run();
  mButtonB.Run();
  mButtonC.Run();
  mTimerFanLEDs.Run();
}
