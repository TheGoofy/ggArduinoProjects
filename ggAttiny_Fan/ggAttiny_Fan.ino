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
#include "ggButton.h"
#include "ggOutput.h"
#include "ggControlPWM.h"
#include "ggTimer.h"
#include "ggFanLEDs.h"
#include "ggFanTacho.h"


// Define pin numbers
#define M_BUTTON_ON_PIN M_ATTINY_PA0_PIN
#define M_BUTTON_A_PIN M_ATTINY_PA1_PIN
#define M_BUTTON_B_PIN M_ATTINY_PA2_PIN
#define M_BUTTON_C_PIN M_ATTINY_PA3_PIN
#define M_FAN_TACHO_PIN M_ATTINY_PA7_PIN
#define M_FAN_PWM_PIN M_ATTINY_PA6_PIN
#define M_LED_DATA_PIN M_ATTINY_PB2_PIN
#define M_EN_5V_PIN M_ATTINY_PB0_PIN
#define M_EN_12V_PIN M_ATTINY_PB1_PIN


// Fan LEDs animation speed
#define M_ANIMATION_LEDs_FPS (36)
void AnimateLEDs();


// periphery + main objects
ggButtonT<M_BUTTON_ON_PIN, true, true, 30, 0> mButtonOn; // inverted, pull-up, 30ms debounce, no repeat
ggButtonT<M_BUTTON_A_PIN, true> mButtonA; // inverted
ggButtonT<M_BUTTON_B_PIN, true> mButtonB; // inverted
ggButtonT<M_BUTTON_C_PIN, true> mButtonC; // inverted
ggOutputT<M_EN_5V_PIN, false> mEnable5V; // not inverted
ggOutputT<M_EN_12V_PIN, false> mEnable12V; // not inverted
ggControlPWM<M_FAN_PWM_PIN, 25000, true> mFanPWM; // 25kHz, inverted
ggFanLEDs<M_LED_DATA_PIN, 3, 12, 5+6> mFanLEDs; // 3 fans, 12 LEDs per fan, led #5 ist first
ggFanTacho<M_FAN_TACHO_PIN, 16> mFanTacho; // 32 FIR-Filter
ggTimerT<1000000 / M_ANIMATION_LEDs_FPS, AnimateLEDs> mAnimationLEDs; // timer for LED animation


namespace ggAnimation {

enum class tType {
  eNone,
  eDimUp,
  eDimDown,
  eSlideUp,
  eSlideDown
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


// At startup the power-button is "pressed". Therefore we'll have a "release" event, which must
// not yet turn off the power. We need to wait, until the button is pressed again (power-off
// armed). Only on this (second) "release" event, we shall turn off the 5V power.
void SetupButtonOn()
{
  static bool vPowerOffArmed = false;
  mButtonOn.Begin();
  mButtonOn.OnPressed([]() {
    // switch on fan- and led-power (when powered by ISP)
    mEnable12V.SetPinModeOutput();
    mEnable5V.SetPinModeOutput();
    mEnable12V.Write(true);
    mEnable5V.Write(true);
    // arm power off for button next release-event
    vPowerOffArmed = !vPowerOffArmed;
  });
  mButtonOn.OnReleased([]() {
    if (vPowerOffArmed) {
      // switch off fan- and led-power
      mFanPWM.SetDutyCycle(mFanPWM.GetDutyCycleMin());
      mEnable12V.SetPinModeInput();
      mEnable5V.SetPinModeInput();
      // Wait some time until 5V power-off () ...
      delay(1000);
      // We're potentially still running here (when powered by ISP).
      // In that case the power-button just switches on or off the 5V and 12V supply
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
    if (mFanPWM.IncDutyCycle(vStepPWM)) {
      ggAnimation::SetType(ggAnimation::tType::eSlideUp);
    }
  });
  mButtonB.OnPressed([]() {
    ggAnimation::SetType(ggAnimation::tType::eDimUp);
  });
  mButtonC.OnPressed([]() {
    if (mFanPWM.DecDutyCycle(vStepPWM)) {
      ggAnimation::SetType(ggAnimation::tType::eSlideDown);
    }
  });
}


void setup()
{
  // on startup first enable 5V step-up converter:
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
