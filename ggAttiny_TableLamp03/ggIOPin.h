#pragma once

class ggIOPin {

public:

  ggIOPin(byte aPin)
  : mPin(aPin) {    
  }

  inline byte GetPin() const {
    return mPin;
  }

private:

  const byte mPin;
  
};
