#include "ggController.h"


ggController::ggController()
: mMode(eModeOff),
  mReferenceValue(20.0f),
  mHysteresisValue(1.0f),
  mOutputAnalog(false),
  mInputValid(true),
  mInputValue(0.0f),
  mOutputValue(0.0f),
  mOutputChangedFunc(nullptr)
{
}


void ggController::Begin()
{
}


ggController::tMode ggController::GetMode() const
{
  return mMode.Get();
}


void ggController::SetMode(tMode aMode)
{
  if (mMode.Get() != aMode) {
    mMode.Set(aMode);
    ControlOutput();
  }
}


float ggController::GetReference() const
{
  return mReferenceValue.Get();
}


void ggController::SetReference(float aReferenceValue)
{
  if (mReferenceValue.Get() != aReferenceValue) {
    mReferenceValue.Set(aReferenceValue);
    ControlOutput();
  }
}


bool ggController::GetInputValid() const
{
  return mInputValid;
}


void ggController::SetInputValid(bool aValid)
{
  if (mInputValid != aValid) {
    mInputValid = aValid;
    ControlOutput();
  }
}


float ggController::GetInput() const
{
  return mInputValue;
}


void ggController::SetInput(float aInputValue)
{
  if (mInputValue != aInputValue) {
    mInputValue = aInputValue;
    ControlOutput();
  }
}


float ggController::GetOutput() const
{
  return mOutputValue;
}


void ggController::OnOutputChanged(tOutputChangedFunc aOutputChangedFunc)
{
  mOutputChangedFunc = aOutputChangedFunc;
}


void ggController::Print(Stream& aStream) const
{
  aStream.printf("mMode = %s\n", ToString(mMode.Get()).c_str());
  aStream.printf("mReferenceValue = %f\n", mReferenceValue.Get());
  aStream.printf("mHysteresisValue = %f\n", mHysteresisValue.Get());
  aStream.printf("mOutputAnalog = %d\n", mOutputAnalog.Get());
  aStream.printf("mInputValue = %f\n", mInputValue);
  aStream.printf("mOutputValue = %f\n", mOutputValue);
}


void ggController::ControlOutput()
{
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


template <>
String ToString(const ggController::tMode& aMode)
{
  switch (aMode) {
    case ggController::eModeOff: return "eModeOff";
    case ggController::eModeOnBelow: return "eModeOnBelow";
    case ggController::eModeOnAbove: return "eModeOnAbove";
    case ggController::eModeOn: return "eModeOn";
    default: return "eModeUnknown";
  }
}


template <>
ggController::tMode FromString(const String& aString)
{
  if (aString == "eModeOff") return ggController::eModeOff;
  if (aString == "eModeOnBelow") return ggController::eModeOnBelow;
  if (aString == "eModeOnAbove") return ggController::eModeOnAbove;
  if (aString == "eModeOn") return ggController::eModeOn;
  return ggController::eModeOff;
}

