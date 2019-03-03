#pragma once

#include <functional>
#include <Stream.h>

#include "ggValueEEPromT.h"
#include "ggStringConvertNumbers.h"

class ggController {

public:

  ggController();

  void Begin();

  enum tMode {
    eModeOff = 0,
    eModeOnBelow = 1,
    eModeOnAbove = 2,
    eModeOn = 3
  };

  tMode GetMode() const;
  void SetMode(tMode aMode);
  float GetReference() const;
  void SetReference(float aReferenceValue);

  bool GetInputValid() const;
  void SetInputValid(bool aValid);
  float GetInput() const;
  void SetInput(float aInputValue);
  float GetOutput() const;
    
  typedef std::function<void(float aOutputValue)> tOutputChangedFunc;
  void OnOutputChanged(tOutputChangedFunc aOutputChangedFunc);

  void Print(Stream& aStream) const;

private:

  void ControlOutput();

  // persistent settings
  ggValueEEPromT<tMode> mMode;
  ggValueEEPromT<float> mReferenceValue;
  ggValueEEPromT<float> mHysteresisValue;
  ggValueEEPromT<bool> mOutputAnalog;

  // variable controls
  bool mInputValid;
  float mInputValue;
  float mOutputValue;

  // callback when putput value changed
  tOutputChangedFunc mOutputChangedFunc;
  
};

template <>
String ToString(const ggController::tMode& aMode);

template <>
ggController::tMode FromString(const String& aString);

