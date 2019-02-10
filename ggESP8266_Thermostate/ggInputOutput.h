#pragma once

class ggInputOutput {

public:

  ggInputOutput(int aPin)
  : mPin(aPin) {    
  }

  int GetPin() const {
    return mPin;
  }

protected:

  int mPin;
  
};

