#pragma once

/**
 * @brief A debounced button handler class for handling button presses and releases.
 *
 * This class handles the reading of a button's state with debouncing to avoid 
 * detecting false presses caused by mechanical noise. It provides methods to 
 * set callback functions that are invoked when the button is pressed or released.
 */
template<uint8_t TPin, // Pin number where the button is connected
         unsigned long TDebounceMillis = 30, // Debounce delay in milliseconds (default is 30 ms)
         bool TInverted = true, // If true, the button logic is inverted (default: pressed = low-input)
         bool TEnablePullUp = true, // If true, enables the internal pull-up resistor (default is true)
         unsigned long TInitialRepeatDelay = 500, // Initial delay before repeat starts (in milliseconds)
         unsigned long TRepeatInterval = 100> // Interval between repeated calls (in milliseconds)
class ggButtonT
{
  public:

    typedef void (*tFunc)();

    ggButtonT() {
    }

    void Begin() {
      // configure pin
      pinMode(TPin, TEnablePullUp ? INPUT_PULLUP : INPUT);
      // remember actual state (suppress ghost-event)
      mPressed = ReadPressed();
      mPressedBounce = mPressed;
      mMillisBounce = millis();
      mMillisLastRepeat = mMillisBounce;
    }

    void OnPressed(tFunc aPressedFunc) {
      mPressedFunc = aPressedFunc;
    }

    void OnReleased(tFunc aReleasedFunc) {
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
      unsigned long vMillis = millis();
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
            mPressedFunc();
          }
          if (!mPressed && mReleasedFunc) {
            mReleasedFunc();
          }
        }
        else if (mPressed && mPressedFunc) {
          // Handle repeated OnPressed callback if the button is held down
          unsigned long vRepeatDelay = (mMillisLastRepeat == mMillisBounce) ? TInitialRepeatDelay : TRepeatInterval;
          if (vMillis - mMillisLastRepeat >= vRepeatDelay) {
            mPressedFunc();
            mMillisLastRepeat = vMillis;
          }
        }          
      }
    }

  private:

    inline bool ReadPressed() const {
      return digitalRead(TPin) ^ TInverted;
    }

    bool mPressed = false; // Current stable state of the button
    bool mPressedBounce = false; // Last "bouncy" state of the button
    unsigned long mMillisBounce = 0; // Time of the last "bouncy" state change
    unsigned long mMillisLastRepeat = 0; // Time of the last repeat callback invocation
    tFunc mPressedFunc = nullptr; // Callback function for button press
    tFunc mReleasedFunc = nullptr; // Callback function for button release
};