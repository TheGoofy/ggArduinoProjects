#include "ggController.h"

#include "ggAlgorithm.h"


ggController::ggController()
: mMode(eModeOff),
  mSetPointValue(20.0f),
  mHysteresisValue(1.0f),
  mOutputAnalog(false),
  mInputValid(true),
  mInputValue(0.0f),
  mOutputValue(0.0f),
  mAutoPID(nullptr),
  mInputValuePID(0.0),
  mSetPointValuePID(0.0),
  mOutputValuePID(0.0),
  mControlP(1.0f),
  mControlI(0.0f),
  mControlD(0.0f),
  mOutputChangedFunc(nullptr)
{
  mAutoPID = new AutoPID(&mInputValuePID,
                         &mSetPointValuePID,
                         &mOutputValuePID, 0.0, 1.0, // out, out-min, out-max
                         1.0, 0.0, 0.0); // KP, KI, KD
}


ggController::~ggController()
{
  delete mAutoPID;
}


void ggController::ResetSettings()
{
  SetMode(eModeOff);
  SetSetPoint(20.0f);
  SetHysteresis(1.0f);
  SetPID(1.0f, 0.0f, 0.0f);
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


float ggController::GetSetPoint() const
{
  return mSetPointValue.Get();
}


void ggController::SetSetPoint(float aSetPoint)
{
  if (mSetPointValue.Get() != aSetPoint) {
    mSetPointValue.Set(aSetPoint);
    mSetPointValuePID = aSetPoint;
    mAutoPID->reset();
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
    mAutoPID->setBangBang(aHysteresis / 2.0f);
    mAutoPID->reset();
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
    mInputValuePID = aInput;
    ControlOutput();
  }
}


float ggController::GetOutput() const
{
  return mOutputValue;
}


const float& ggController::GetP() const
{
  return mControlP.Get();
}


const float& ggController::GetI() const
{
  return mControlI.Get();
}


const float& ggController::GetD() const
{
  return mControlD.Get();
}


void ggController::SetPID(float aP, float aI, float aD)
{
  mControlP.Set(aP);
  mControlI.Set(aI);
  mControlD.Set(aD);
  mAutoPID->setGains(aP, aI, aD);
  mAutoPID->reset();
}


void ggController::OnOutputChanged(tOutputChangedFunc aOutputChangedFunc)
{
  mOutputChangedFunc = aOutputChangedFunc;
}


void ggController::Run()
{
  ControlOutput();
}


void ggController::Print(Stream& aStream) const
{
  aStream.printf("ggController::mMode = %s\n", ToString(mMode.Get()).c_str());
  aStream.printf("ggController::mInputValue = %f\n", mInputValue);
  aStream.printf("ggController::mSetPointValue = %f\n", mSetPointValue.Get());
  aStream.printf("ggController::mHysteresisValue = %f\n", mHysteresisValue.Get());
  aStream.printf("ggController::mPID = %f / %f / %f\n", mControlP.Get(), mControlI.Get(), mControlD.Get());
  aStream.printf("ggController::mOutputAnalog = %d\n", mOutputAnalog.Get());
  aStream.printf("ggController::mOutputValue = %f\n", mOutputValue);
}


void ggController::ControlOutput()
{
  // output is "off" by default (safety)
  float vOutputValue = 0.0f;
  switch (GetMode()) {
    case eModeOn: vOutputValue = 1.0f; break;
    case eModeOff: vOutputValue = 0.0f; break;
    case eModeOnBelow: if (mInputValid) ControlOutput(true, vOutputValue); break;
    case eModeOnAbove: if (mInputValid) ControlOutput(false, vOutputValue); break;
    case eModePID: if (mInputValid) ControlOutputPID(vOutputValue); break;
    default: vOutputValue = 0.0f; break;
  }

  // "update" doesn't notify if output value unchanged
  UpdateOutput(vOutputValue);
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
  float vDifference = aInverted ? (GetSetPoint() - mInputValue)
                                : (mInputValue - GetSetPoint());
                                
  // use inverse hysteresis as p-control-value
  if (GetHysteresis() != 0.0f) {
    float vOutput = vDifference / GetHysteresis() + 0.5f;
    aOutput = ggClamp<float>(vOutput, 0.0f, 1.0f);
  }
  else {
    aOutput = vDifference > 0.0f ? 1.0f : 0.0f;
  }
}


void ggController::ControlOutputDigital(bool aInverted, float& aOutput) const
{
  // calculate hysteresis range
  const float vHysteresisHalf = GetHysteresis() / 2.0f;
  const float vValueMin = GetSetPoint() - vHysteresisHalf;
  const float vValueMax = GetSetPoint() + vHysteresisHalf;
  
  // compare input and hysteresis range
  if (mInputValue <= vValueMin) {
    // input is below hysteresis range
    aOutput = aInverted ? 1.0f : 0.0f;
  }
  else if (mInputValue >= vValueMax) {
    // input is above hysteresis range
    aOutput = aInverted ? 0.0f : 1.0f;
  }
  else {
    // input is within hysteresis range (keep output as it is)
    aOutput = mOutputValue;
  }
}


void ggController::ControlOutputPID(float& aOutput) const
{
  mAutoPID->run();
  aOutput = mOutputValuePID;
}


void ggController::UpdateOutput(float aOutput)
{
  if (mOutputValue != aOutput) {
    mOutputValue = aOutput;
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
    case ggController::eModeOn: return "eModeOn";
    case ggController::eModeOnBelow: return "eModeOnBelow";
    case ggController::eModeOnAbove: return "eModeOnAbove";
    case ggController::eModePID: return "eModePID";
    default: return "eModeUnknown";
  }
}


template <>
ggController::tMode FromString(const String& aString)
{
  if (aString == "eModeOff") return ggController::eModeOff;
  if (aString == "eModeOn") return ggController::eModeOn;
  if (aString == "eModeOnBelow") return ggController::eModeOnBelow;
  if (aString == "eModeOnAbove") return ggController::eModeOnAbove;
  if (aString == "eModePID") return ggController::eModePID;
  return ggController::eModeOff;
}
