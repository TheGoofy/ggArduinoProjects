#pragma once

class ggInputOutput {

public:

  ggInputOutput(int aPin)
  : mPin(aPin) {    
  }

  int GetPin() const {
    return mPin;
  }

  void Print(Stream& aStream) const {
    aStream.printf("ggInputOutput::mPin = %d\n", mPin);
  }
  
private:

  const int mPin;
  
};
