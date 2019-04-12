#include "ggController.h"

#include "ggAlgorithm.h"


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


void ggController::ResetSettings()
{
  SetMode(eModeOff);
  SetReference(20.0f);
  SetHysteresis(1.0f);
  SetOutputAnalog(false);
  ControlOutput();
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


void ggController::SetReference(float aReference)
{
  if (mReferenceValue.Get() != aReference) {
    mReferenceValue.Set(aReference);
    ControlOutput();
  }
}


float ggController::GetHysteresis() const
{
  return mHysteresisValue.Get();
}


void ggController::SetHysteresis(float aHysteresis)
{
  if (mHysteresisValue.Get() != aHysteresis) {
    mHysteresisValue.Set(aHysteresis);
    ControlOutput();
  }
}


bool ggController::GetOutputAnalog() const
{
  return mOutputAnalog.Get();
}


void ggController::SetOutputAnalog(bool aOutputAnalog)
{
  if (mOutputAnalog.Get() != aOutputAnalog) {
    mOutputAnalog.Set(aOutputAnalog);
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


void ggController::SetInput(float aInput)
{
  if (mInputValue != aInput) {
    mInputValue = aInput;
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
  // output is "off" by default (safety)
  float vOutputValue = 0.0f;
  switch (GetMode()) {
    case eModeOff: vOutputValue = 0.0f; break;
    case eModeOnBelow: if (mInputValid) ControlOutput(true, vOutputValue); break;
    case eModeOnAbove: if (mInputValid) ControlOutput(false, vOutputValue); break;
    case eModeOn: vOutputValue = 1.0f; break;
    default: vOutputValue = 0.0f; break;
  }

  // don't do anything if nocthing changed
  if (mOutputValue != vOutputValue) {
    mOutputValue = vOutputValue;
    if (mOutputChangedFunc != nullptr) {
      mOutputChangedFunc(mOutputValue);
    }
  }
}


void ggController::ControlOutput(bool aInverted, float& aOutput) const
{
  if (GetOutputAnalog()) {
    ControlOutputAnalog(aInverted, aOutput);
  }
  else {
    ControlOutputDigital(aInverted, aOutput);
  }
}


void ggController::ControlOutputAnalog(bool aInverted, float& aOutput) const
{
  // calculate difference between input and reference
  float vDifference = aInverted ? (GetReference() - mInputValue)
                                : (mInputValue - GetReference());
                                
  // use inverse hysteresis as p-control-value
  if (GetHysteresis() != 0.0f) {
    float vOutput = vDifference / GetHysteresis();
    aOutput = ggClamp<float>(vOutput + 0.5f, 0.0f, 1.0f);
  }
  else {
    aOutput = vDifference > 0.0f ? 1.0f : 0.0f;
  }
}


void ggController::ControlOutputDigital(bool aInverted, float& aOutput) const
{
  // calculate hysteresis range
  const float vHysteresisHalf = GetHysteresis() / 2.0f;
  const float vReferenceMin = GetReference() - vHysteresisHalf;
  const float vReferenceMax = GetReference() + vHysteresisHalf;
  
  // compare input and hysteresis range
  if (mInputValue <= vReferenceMin) {
    // input is below hysteresis range
    aOutput = aInverted ? 1.0f : 0.0f;
  }
  else if (mInputValue >= vReferenceMax) {
    // input is above hysteresis range
    aOutput = aInverted ? 0.0f : 1.0f;
  }
  else {
    // input is within hysteresis range (keep output as it is)
    aOutput = mOutputValue;
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

