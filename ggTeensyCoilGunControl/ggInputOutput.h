#pragma once

#include "ggDebug.h"

class ggInputOutput {

public:

  ggInputOutput(int aPin)
  : mPin(aPin) {    
  }

  int GetPin() const {
    return mPin;
  }

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggInputOutput", aName);
    vDebug.PrintF("mPin = %d\n", mPin);
    /*
    const char* vPinMode = "unknown";
    switch (getPinMode(mPin)) {
      case INPUT: vPinMode = "INPUT"; break;
      case INPUT_PULLUP: vPinMode = "INPUT_PULLUP"; break;
      case OUTPUT: vPinMode = "OUTPUT"; break;
    }
    vDebug.PrintF("PinMode = %s\n", vPinMode);
    */
  }
  
private:

  #define UNKNOWN_PIN 0xFF
/*
  uint8_t getPinMode(uint8_t pin) const {
    uint8_t bit = digitalPinToBitMask(pin);
    uint8_t port = digitalPinToPort(pin);

    // I don't see an option for mega to return this, but whatever...
    if (NOT_A_PIN == port) return UNKNOWN_PIN;

    // Is there a bit we can check?
    if (0 == bit) return UNKNOWN_PIN;

    // Is there only a single bit set?
    if (bit & bit - 1) return UNKNOWN_PIN;

    volatile uint32_t *reg, *out;
    reg = portModeRegister(port);
    out = portOutputRegister(port);

    if (*reg & bit)
      return OUTPUT;
    else if (*out & bit)
      return INPUT_PULLUP;
    else
      return INPUT;
  }
*/
  const int mPin;
  
};
