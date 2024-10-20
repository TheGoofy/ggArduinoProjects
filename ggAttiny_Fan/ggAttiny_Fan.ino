/*
Configuration for ATtiny84a:
- Tools / Board / ATTinyCore / ATtiny24/44/84(a) (no bootloader)
- Tools / B.O.D Level 4.3V (without BOD, the ATtiny restarts by accident)
- Tools / Chip: ATtiny84(a)
- Tools / Clock 8 MHz internal (needed for 800kHz LED-Communication, 4MHz would be too slow)
- Tools / Programmer: "Arduino as ISP"
- Tools / Burn Bootloader (needed for special settings like 8MHz internal, BOD)
- Sketch / Upload using Programmer
*/

#include "ggAttinyPins.h"
#include "ggTimer.h"
#include "ggInput.h"
#include "ggInputAnalog.h"
#include "ggButton.h"
#include "ggOutput.h"
#include "ggControlPWM.h"
#include "ggTimer.h"
#include "ggFanLEDs.h"
#include "ggFanTacho.h"
#include "ggMorse.h"


// Define pin numbers
#define GG_BUTTON_ON_PIN GG_ATTINY_PA0_PIN
#define GG_BUTTON_A_PIN GG_ATTINY_PA1_PIN
#define GG_BUTTON_B_PIN GG_ATTINY_PA2_PIN
#define GG_BUTTON_C_PIN GG_ATTINY_PA3_PIN
#define GG_VBAT_PIN GG_ATTINY_PA4_PIN // aka ADC4
#define GG_FAN_TACHO_PIN GG_ATTINY_PA7_PIN
#define GG_FAN_PWM_PIN GG_ATTINY_PA6_PIN
#define GG_LED_DATA_PIN GG_ATTINY_PB2_PIN
#define GG_EN_5V_PIN GG_ATTINY_PB0_PIN
#define GG_EN_12V_PIN GG_ATTINY_PB1_PIN


// Battery monitor
#define GG_BATTERY_MONITOR_INTERVAL_MS (100L) // interval in milli-seconds
#define GG_BATTERY_MONITOR_FILTER_FIR (8L) // FIR average some values
#define GG_BATTERY_MONITOR_MV_LOW   3600L // 3.60V => approximate charge state 10%
#define GG_BATTERY_MONITOR_MV_EMPTY 3400L // 3.40V => approximate charge state below 5%
void BatteryMonitor();


// Fan LEDs animation speed
#define GG_ANIMATION_LEDs_FPS (36) // frames per second
void AnimateLEDs();


// periphery + main objects
ggButtonT<GG_BUTTON_ON_PIN, true, true, 30, 0> mButtonOn; // inverted, pull-up, 30ms debounce, no repeat
ggButtonT<GG_BUTTON_A_PIN, true> mButtonA; // inverted
ggButtonT<GG_BUTTON_B_PIN, true> mButtonB; // inverted
ggButtonT<GG_BUTTON_C_PIN, true> mButtonC; // inverted
ggInputAnalogT<GG_VBAT_PIN, DEFAULT, 5000, -56> mBatteryVoltage; // us VCC (aka DEFAULT) as voltage reference, 5000 mV
ggOutputT<GG_EN_5V_PIN, false> mEnable5V; // not inverted
ggOutputT<GG_EN_12V_PIN, false> mEnable12V; // not inverted
ggControlPWM<GG_FAN_PWM_PIN, 25000, true> mFanPWM; // 25kHz, inverted
ggFanLEDs<GG_LED_DATA_PIN, 3, 12, 5+6> mFanLEDs; // 3 fans, 12 LEDs per fan, led #5 ist first
ggFanTacho<GG_FAN_TACHO_PIN, 16> mFanTacho; // 32 FIR-Filter
ggTimerT<1000L * GG_BATTERY_MONITOR_INTERVAL_MS, BatteryMonitor> mMonitorBattery; // timer for monitoring battery voltage
ggTimerT<1000000L / GG_ANIMATION_LEDs_FPS, AnimateLEDs> mAnimationLEDs; // timer for LED animation


void PowerOn()
{
  // switch on fan- and led-power (when powered by ISP)
  mEnable5V.SetPinModeOutput();
  mEnable12V.SetPinModeOutput();
  mEnable5V.Write(true);
  mEnable12V.Write(true);
}


void PowerOff()
{
   // switch off fan- and led-power
   // note: external pull-down resistors will interrupt battery supply
   mEnable12V.SetPinModeInput();
   mEnable5V.SetPinModeInput();
   // Wait some time until 5V power-off () ...
   delay(1000);
}


namespace ggAnimation {

enum class tType {
  eNone,
  eDimUp,
  eDimDown,
  eSlideUp,
  eSlideDown,
  eBattLow
};

tType mType = tType::eNone;
int8_t mSlidePosY = 0;
uint32_t mColor = 0;
uint8_t mBrightness = 0;

constexpr uint8_t GetColorR(uint32_t aColor) {
  return static_cast<uint8_t>(aColor >> 16);
}

constexpr uint8_t GetColorG(uint32_t aColor) {
  return static_cast<uint8_t>(aColor >> 8);
}

constexpr uint8_t GetColorB(uint32_t aColor) {
  return static_cast<uint8_t>(aColor >> 0);
}

constexpr uint32_t GetColor(uint8_t aR, uint8_t aG, uint8_t aB) {
  return (static_cast<uint32_t>(aR) << 16) | (static_cast<uint32_t>(aG) << 8) | (static_cast<uint32_t>(aB) << 0);
}

constexpr uint8_t Multiply8(uint8_t aValueA, uint8_t aValueB) {
  return static_cast<uint8_t>((static_cast<uint16_t>(aValueA) * static_cast<uint16_t>(aValueB)) >> 8);
}

constexpr uint32_t ScaleColor(uint8_t aValue, uint32_t aColor) {
  return GetColor(Multiply8(aValue, GetColorR(aColor)),
                  Multiply8(aValue, GetColorG(aColor)),
                  Multiply8(aValue, GetColorB(aColor)));
}

tType GetType() {
  return mType;
}

void SetType(tType aType) {
  if (mType != aType) {
    switch (aType) {
      case tType::eSlideUp:
        mSlidePosY = 0;
        mColor = GetColor(10, 0, 100);
        mBrightness = 255;
        break;
      case tType::eSlideDown:
        mSlidePosY = mFanLEDs.GetSizeY_V() - 1;
        mColor = GetColor(100, 0, 10);
        mBrightness = 255;
        break;
      case tType::eDimUp:
        mSlidePosY = mFanLEDs.GetSizeY_V() / 2 - 1;
        mColor = Adafruit_NeoPixel::ColorHSV(2 * rand());
        mBrightness = 255;
        break;
      case tType::eBattLow:
        mColor = GetColor(30, 15, 0);
        mBrightness = 255;
        break;
      default:
        break;
    }
    mType = aType;
  }
}

void SetPixelsX(uint8_t aIndexY, uint32_t aColor) {
  for (uint8_t vIndexX = 0; vIndexX < mFanLEDs.GetSizeX_V(); vIndexX++) {
    mFanLEDs.SetPixelXY_V(vIndexX, aIndexY, aColor);
  }
}

void Step() {
  switch (mType) {

    case tType::eSlideUp:
      if (mSlidePosY < mFanLEDs.GetSizeY_V()) {
        SetPixelsX(mSlidePosY, GetColor(0, 255, 150));
      }
      if (mSlidePosY > 0) {
        SetPixelsX(mSlidePosY - 1, mColor);
      }
      if (mSlidePosY >= mFanLEDs.GetSizeY_V()) {
        mType = tType::eDimDown;
      }
      mFanLEDs.Show();
      ++mSlidePosY;
      break;

    case tType::eSlideDown:
      if (mSlidePosY >= 0) {
        SetPixelsX(mSlidePosY, GetColor(255, 150, 0));
      }
      if (mSlidePosY < mFanLEDs.GetSizeY_V() - 1) {
        SetPixelsX(mSlidePosY + 1, mColor);
      }
      if (mSlidePosY < 0) {
        mType = tType::eDimDown;
      }
      mFanLEDs.Show();
      --mSlidePosY;
      break;

    case tType::eDimUp:
      if (mSlidePosY >= 0) {
        SetPixelsX(mSlidePosY, mColor);
        SetPixelsX(mFanLEDs.GetSizeY_V() - mSlidePosY - 1, mColor);
      }
      if (mSlidePosY == 0) {
        mType = tType::eDimDown;
      }
      mFanLEDs.Show();
      --mSlidePosY;
      break;
      
    case tType::eDimDown:
    case tType::eBattLow:
      if (mBrightness >= 2) {
        mBrightness -= 2;
        mFanLEDs.Fill(ScaleColor(Adafruit_NeoPixel::gamma8(mBrightness), mColor));
        mFanLEDs.Show();
      }
      else {
        mType = tType::eNone;
      }
      break;

    case tType::eNone:
      break;

    default:
      break;
  }
}

};


void AnimateLEDs()
{
  ggAnimation::Step();
}


#define GG_MORSE_DIT_MILLIS 250L

void MorseSignalOn(uint8_t aNumDits)
{
  mFanLEDs.Fill(aNumDits == 1 ? mFanLEDs.GetColor(100,255,0) : mFanLEDs.GetColor(0,100,255));
  mFanLEDs.Show();
  delay(aNumDits * GG_MORSE_DIT_MILLIS);
}

void MorseSignalOff(uint8_t aNumDits)
{
  mFanLEDs.Fill(mFanLEDs.GetColor(0,0,0));
  mFanLEDs.Show();
  delay(aNumDits * GG_MORSE_DIT_MILLIS);
}

ggMorseT<MorseSignalOn, MorseSignalOff> mMorse;


namespace ggBatteryMonitor {

uint16_t mBatteryMV = 0;

void Display() {
  mMorse.Signal(mBatteryMV);
}

void Check() {

  // use an FIR-Filter for eliminating noise
  constexpr uint32_t vFilterFIR = GG_BATTERY_MONITOR_FILTER_FIR;
  mBatteryMV = ((vFilterFIR - 1) * mBatteryMV + mBatteryVoltage.ReadMV()) / vFilterFIR;

  // wait long enough after power-on to check battery voltage
  if (millis() > 10 * GG_BATTERY_MONITOR_INTERVAL_MS * GG_BATTERY_MONITOR_FILTER_FIR) {
    if (mBatteryMV < GG_BATTERY_MONITOR_MV_EMPTY) {
      PowerOff();
    }
  }

  // indicate low battery
  if (mBatteryMV < GG_BATTERY_MONITOR_MV_LOW) {
    if (ggAnimation::GetType() == ggAnimation::tType::eNone) {
      ggAnimation::SetType(ggAnimation::tType::eBattLow);
    }
  }
}

};


void BatteryMonitor()
{
  ggBatteryMonitor::Check();
}


// At startup the power-button is "pressed". Therefore we'll have a "release" event, which must
// not yet turn off the power. We need to wait, until the button is pressed again (power-off
// armed). Only on this (second) "release" event, we shall turn off the 5V power.
void SetupButtonOn()
{
  static bool vPowerOffArmed = false;
  mButtonOn.Begin();
  mButtonOn.OnPressed([](bool aRepeated) {
    if ((!aRepeated)) {
      // switch on fan- and led-power (when powered by ISP)
      PowerOn();
      // arm power off for button next release-event
      vPowerOffArmed = !vPowerOffArmed;
    }
  });
  mButtonOn.OnReleased([]() {
    if (vPowerOffArmed) {
      // switch off fan- and led-power
      PowerOff();
      // We're potentially still running here (when powered by ISP).
      // In that case the power-button just switches on or off the 5V and 12V supply
    }
  });
}


void SetupButtonsABC()
{
  // configure button pins (as input)
  mButtonA.Begin();
  mButtonB.Begin();
  mButtonC.Begin();
  // use buttons A and B for Fan speed control
  constexpr uint16_t vSteps = 20;
  constexpr uint16_t vStepPWM = (mFanPWM.GetDutyCycleMax() - mFanPWM.GetDutyCycleMin()) / vSteps;
  mButtonA.OnPressed([](bool aRepeated) {
    if (mFanPWM.IncDutyCycle(vStepPWM)) {
      ggAnimation::SetType(ggAnimation::tType::eSlideUp);
    }
  });
  mButtonC.OnPressed([](bool aRepeated) {
    if (mFanPWM.DecDutyCycle(vStepPWM)) {
      ggAnimation::SetType(ggAnimation::tType::eSlideDown);
    }
  });
  // use button B for fun and voltage display
  mButtonB.OnPressed([](bool aRepeated) {
    if (aRepeated) {
      ggBatteryMonitor::Display();
    }
  });
  mButtonB.OnReleased([]() {
    ggAnimation::SetType(ggAnimation::tType::eDimUp);
  }); 
}


void setup()
{
  // battery voltage monitor
  mBatteryVoltage.Begin();

  // on startup first enable 5V step-up converter:
  // - Attiny is powered by these 5V
  // - 5V supply would shutdown again, as soon as on/off button is released
  mEnable5V.Begin();
  mEnable12V.Begin();
  PowerOn();

  // power-button is special
  SetupButtonOn();
  SetupButtonsABC();

  // initialize Fan
  mFanPWM.Begin();
  mFanPWM.SetDutyCycle(mFanPWM.GetDutyCycleMax() / 4);
  mFanLEDs.Begin();
  mFanTacho.Begin();
}


void loop()
{
  mButtonOn.Run();
  mButtonA.Run();
  mButtonB.Run();
  mButtonC.Run();
  mFanTacho.Run();
  mAnimationLEDs.Run();
  mMonitorBattery.Run();
/*
  // turn on and off LEDs for each fan blade ...
  {
    static uint32_t vMicrosLast = micros();
    uint32_t vMicrosDelta = micros() - vMicrosLast;
    const uint32_t vMicrosPerBlade = mFanTacho.GetMicrosPerRev() / 10;
    if (vMicrosDelta >= vMicrosPerBlade) {
      vMicrosLast += vMicrosDelta;
      static bool vShow = true;
      mFanLEDs.Show(vShow);
      vShow = !vShow;
    }
  }
*/
}
