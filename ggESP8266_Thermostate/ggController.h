#pragma once

#include <functional>

class ggController {

public:

  enum tMode {
    eModeOff = 0,
    eModeOnBelow = 1,
    eModeOnAbove = 2,
    eModeOn = 3
  };

  typedef std::function<void(float aOutputValue)> tOutputChangedFunc;

  ggController()
  : mMode(eModeOff),
    mReferenceValue(0.0f),
    mInputValue(0.0f),
    mOutputValue(0.0f),
    mOutputChangedFunc(nullptr) {
  }

  tMode GetMode() const {
    return mMode;
  }

  void SetMode(tMode aMode) {
    if (mMode != aMode) {
      mMode = aMode;
      ControlOutput();
    }
  }

  float GetReference() const {
    return mReferenceValue;
  }

  void SetReference(float aReferenceValue) {
    if (mReferenceValue != aReferenceValue) {
      mReferenceValue = aReferenceValue;
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
    aStream.printf("mMode = %d\n", mMode);
    aStream.printf("mReferenceValue = %f\n", mReferenceValue);
    aStream.printf("mInputValue = %f\n", mInputValue);
    aStream.printf("mOutputValue = %f\n", mOutputValue);
  }

private:

  void ControlOutput() {
    float vOutputValue = 0.0f;
    switch (mMode) {
      case eModeOff: vOutputValue = 0.0f; break;
      case eModeOnBelow: vOutputValue = mInputValue < mReferenceValue ? 1.0f : 0.0f; break;
      case eModeOnAbove: vOutputValue = mInputValue > mReferenceValue ? 1.0f : 0.0f; break;
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

  tMode mMode;
  
  float mReferenceValue;
  float mInputValue;
  float mOutputValue;
  
  tOutputChangedFunc mOutputChangedFunc;
  
};

