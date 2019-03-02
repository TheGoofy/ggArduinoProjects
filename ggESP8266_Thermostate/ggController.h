#pragma once

#include <functional>

#include "ggValueEEPromT.h"
#include "ggStringConvertNumbers.h"

class ggController {

public:

  enum tMode {
    eModeOff = 0,
    eModeOnBelow = 1,
    eModeOnAbove = 2,
    eModeOn = 3
  };

  typedef std::function<void(float aOutputValue)> tOutputChangedFunc;

  ggController(const String& aName = "ggController")
  : mMode(eModeOff),
    mReferenceValue(20.0f),
    mInputValid(true),
    mInputValue(0.0f),
    mOutputValue(0.0f),
    mOutputChangedFunc(nullptr) {
  }

  void Begin() {
  }

  tMode GetMode() const {
    return mMode.Get();
  }

  void SetMode(tMode aMode) {
    if (mMode.Get() != aMode) {
      mMode.Set(aMode);
      ControlOutput();
    }
  }

  bool GetInputValid() const {
    return mInputValid;
  }

  void SetInputValid(boolean aValid) {
    if (mInputValid != aValid) {
      mInputValid = aValid;
      ControlOutput();
    }
  }

  float GetReference() const {
    return mReferenceValue.Get();
  }

  void SetReference(float aReferenceValue) {
    if (mReferenceValue.Get() != aReferenceValue) {
      mReferenceValue.Set(aReferenceValue);
      ControlOutput();
    }
  }

  float GetInput() const {
    return mInputValue;
  }

  void SetInput(float aInputValue) {
    if (mInputValue != aInputValue) {
      mInputValue = aInputValue;
      ControlOutput();
    }
  }

  float GetOutput() const {
    return mOutputValue;
  }

  void OnOutputChanged(tOutputChangedFunc aOutputChangedFunc) {
    mOutputChangedFunc = aOutputChangedFunc;
  }

  void Print(Stream& aStream) const {
    aStream.printf("mMode = %d\n", mMode.Get());
    aStream.printf("mReferenceValue = %f\n", mReferenceValue.Get());
    aStream.printf("mInputValue = %f\n", mInputValue);
    aStream.printf("mOutputValue = %f\n", mOutputValue);
  }

private:

  void ControlOutput() {
    float vOutputValue = 0.0f;
    switch (GetMode()) {
      case eModeOff: vOutputValue = 0.0f; break;
      case eModeOnBelow: vOutputValue = (mInputValid && (mInputValue < GetReference())) ? 1.0f : 0.0f; break;
      case eModeOnAbove: vOutputValue = (mInputValid && (mInputValue > GetReference())) ? 1.0f : 0.0f; break;
      case eModeOn: vOutputValue = 1.0f; break;
      default: vOutputValue = 1.0f; break;
    }
    if (mOutputValue != vOutputValue) {
      mOutputValue = vOutputValue;
      if (mOutputChangedFunc != nullptr) {
        mOutputChangedFunc(mOutputValue);
      }
    }
  }

  ggValueEEPromT<tMode> mMode;
  ggValueEEPromT<float> mReferenceValue;
  bool mInputValid;
  float mInputValue;
  float mOutputValue;
  
  tOutputChangedFunc mOutputChangedFunc;
  
};


template <>
String ToString(const ggController::tMode& aMode) {
  switch (aMode) {
    case ggController::eModeOff: return "eModeOff";
    case ggController::eModeOnBelow: return "eModeOnBelow";
    case ggController::eModeOnAbove: return "eModeOnAbove";
    case ggController::eModeOn: return "eModeOn";
    default: return "eModeUnknown";
  }
}


template <>
ggController::tMode FromString(const String& aString) {
  if (aString == "eModeOff") return ggController::eModeOff;
  if (aString == "eModeOnBelow") return ggController::eModeOnBelow;
  if (aString == "eModeOnAbove") return ggController::eModeOnAbove;
  if (aString == "eModeOn") return ggController::eModeOn;
  return ggController::eModeOff;
}
