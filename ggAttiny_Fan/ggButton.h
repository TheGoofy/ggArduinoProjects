#pragma once

#include "ggInput.h"

/**
 * @brief A debounced button handler class for handling button presses and releases.
 *
 * This class handles the reading of a button's state with debouncing to avoid 
 * detecting false presses caused by mechanical noise. It provides methods to 
 * set callback functions that are invoked when the button is pressed or released.
 */
template<uint8_t TPin, // Pin number where the button is connected
         bool TInverted = true, // If true, the button logic is inverted (default: pressed = low-input)
         bool TEnablePullUp = true, // If true, enables the internal pull-up resistor (default is true)
         uint32_t TDebounceMillis = 30, // Debounce delay in milliseconds (default is 30 ms)
         uint32_t TInitialRepeatDelay = 500, // Initial delay before repeat starts (in milliseconds), 0 deactivates "repeat"
         uint32_t TRepeatInterval = 100> // Interval between repeated calls (in milliseconds)
class ggButtonT : public ggInputT<TPin, TInverted, TEnablePullUp>
{
  public:

    typedef void (*tPressedFunc)(bool aRepeated);
    typedef void (*tReleasedFunc)();

    ggButtonT() {
    }

    void Begin() {
      // configure pin
      ggInputT<TPin, TInverted, TEnablePullUp>::Begin();
      // pinMode(TPin, TEnablePullUp ? INPUT_PULLUP : INPUT);
      // remember actual state (suppress ghost-event)
      mPressed = ReadPressed();
      mPressedBounce = mPressed;
      mMillisBounce = millis();
      mMillisLastRepeat = mMillisBounce;
    }

    void OnPressed(tPressedFunc aPressedFunc) {
      mPressedFunc = aPressedFunc;
    }

    void OnReleased(tReleasedFunc aReleasedFunc) {
      mReleasedFunc = aReleasedFunc;
    }

    inline bool IsPressed() const {
      return mPressed;
    }

    inline bool IsReleased() const {
      return !mPressed;
    }

    void Run() {
      // get current time and current button state
      uint32_t vMillis = millis();
      bool vPressed = ReadPressed();
      // Check if the button state has changed since the last read
      if (vPressed != mPressedBounce) {
        mPressedBounce = vPressed;
        mMillisBounce = vMillis;
      }
      // Check if button state was stable (not bouncing) long enough
      if (vMillis - mMillisBounce > TDebounceMillis) {
        // Check if new stable state is different than the previous stable state
        if (vPressed != mPressed) {
          // Remember new stable state
          mPressed = vPressed;
          mMillisLastRepeat = mMillisBounce; // Reset the repeat timer
          if (mPressed && mPressedFunc) {
            mPressedFunc(false);
          }
          if (!mPressed && mReleasedFunc) {
            mReleasedFunc();
          }
        }
        else if (mPressed && mPressedFunc && TInitialRepeatDelay > 0) {
          // Handle repeated OnPressed callback if the button is held down
          uint32_t vRepeatDelay = (mMillisLastRepeat == mMillisBounce) ? TInitialRepeatDelay : TRepeatInterval;
          if (vMillis - mMillisLastRepeat >= vRepeatDelay) {
            mPressedFunc(true);
            mMillisLastRepeat = vMillis;
          }
        }          
      }
    }

  private:

    inline bool ReadPressed() const {
      return ggInputT<TPin, TInverted, TEnablePullUp>::Read();
    }

    bool mPressed = false; // Current stable state of the button
    bool mPressedBounce = false; // Last "bouncy" state of the button
    uint32_t mMillisBounce = 0; // Time of the last "bouncy" state change
    uint32_t mMillisLastRepeat = 0; // Time of the last repeat callback invocation
    tPressedFunc mPressedFunc = nullptr; // Callback function for button press
    tReleasedFunc mReleasedFunc = nullptr; // Callback function for button release
};