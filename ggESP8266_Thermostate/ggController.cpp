#include "ggController.h"

#include "ggAlgorithm.h"
#include "ggDebug.h"


ggController::ggController()
: mMode(eModeOff),
  mSetPointValue(20.0f),
  mHysteresisValue(1.0f),
  mOutputAnalog(false),
  mInputValid(true),
  mInputValue(0.0f),
  mOutputValue(0.0f),
  mSamplerPID(0.2f), // 0.2Hz (5s sampling time)
  mControlP(0.35f),
  mControlI(0.004f),
  mControlD(13.0f),
  mOutputMin(0.0f),
  mOutputMax(1.0f),
  mMicrosLast(0),
  mErrorLast(0.0f),
  mErrorI(0.0f),
  mOutputChangedFunc(nullptr)
{
}


ggController::~ggController()
{
}


void ggController::ResetSettings()
{
  SetMode(eModeOff);
  SetSetPoint(20.0f);
  SetHysteresis(1.0f);
  SetPID(0.4f, 0.004f, 12.0f);
  SetOutputAnalog(false);
  ResetControlStatePID();
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
    ResetControlStatePID();
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
}


void ggController::OnOutputChanged(tOutputChangedFunc aOutputChangedFunc)
{
  mOutputChangedFunc = aOutputChangedFunc;
}


void ggController::Begin()
{
  ResetControlStatePID();
  mSamplerPID.OnSample([&] () {
    ControlOutput();
  });
}


void ggController::Run()
{
  mSamplerPID.Run();
}


void ggController::PrintDebug(const String& aName) const
{
  ggDebug vDebug("ggController", aName);
  vDebug.PrintF("mMode = %s\n", ToString(mMode.Get()).c_str());
  vDebug.PrintF("mInputValue = %f\n", mInputValue);
  vDebug.PrintF("mSetPointValue = %f\n", mSetPointValue.Get());
  vDebug.PrintF("mHysteresisValue = %f\n", mHysteresisValue.Get());
  vDebug.PrintF("mPID = %f / %f / %f\n", mControlP.Get(), mControlI.Get(), mControlD.Get());
  vDebug.PrintF("mOutputAnalog = %d\n", mOutputAnalog.Get());
  vDebug.PrintF("mOutputValue = %f\n", mOutputValue);
  vDebug.PrintF("mOutputMin = %f\n", mOutputMin);
  vDebug.PrintF("mOutputMax = %f\n", mOutputMax);
  vDebug.PrintF("mMicrosLast = %ul\n", mMicrosLast);
  vDebug.PrintF("mErrorLast = %f\n", mErrorLast);
  vDebug.PrintF("mErrorI = %f\n", mErrorI);
}


void ggController::ResetControlStatePID()
{
  mMicrosLast = micros();
  mErrorLast = 0.0f;
  mErrorI = 0.0f;
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
  // calculate time since last sample
  unsigned long vMicros = micros();
  float vTimeDelta = (vMicros - mMicrosLast) / 1000000.0f;

  // calculate current error
  float vError = mSetPointValue.Get() - mInputValue;

  // "zero" output is in the middle of the output range
  float vOutput = (mOutputMin + mOutputMax) / 2.0f;

  // P-control (proportional error)
  vOutput += mControlP.Get() * vError;

  // D-control (differential error)
  float vErrorD = (vError - mErrorLast) / vTimeDelta;
  vOutput += mControlD.Get() * vErrorD;

  // I-control (integral error)
  if (mControlI.Get() != 0.0f) {
    if (vError * vErrorD >= 0.0f) {
      // only sum up integral error, input moves away from set-point
      mErrorI += (vError + mErrorLast) / 2.0f * vTimeDelta; // mid-point riemann sum
    }
    // avoid windup of integral error, when output gets at its limits
    vOutput = ggClamp(vOutput, mOutputMin, mOutputMax);
    float vErrorIMin = (mOutputMin - vOutput) / mControlI.Get();
    float vErrorIMax = (mOutputMax - vOutput) / mControlI.Get();
    mErrorI = ggClamp(mErrorI, vErrorIMin, vErrorIMax);
    vOutput += mControlI.Get() * mErrorI;
  }

  // clamp output into range
  aOutput = ggClamp(vOutput, mOutputMin, mOutputMax);

  // update "last" values for next sampling
  mMicrosLast = vMicros;
  mErrorLast = vError;
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
