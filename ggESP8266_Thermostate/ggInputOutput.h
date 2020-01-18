#pragma once

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
  }
  
private:

  const int mPin;
  
};
