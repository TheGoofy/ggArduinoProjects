#pragma once

class ggInputOutput {

public:

  ggInputOutput(int aPin)
  : mPin(aPin) {    
  }

  int GetPin() const {
    return mPin;
  }

private:

  const int mPin;
  
};

