#include "ggController.h"

#include "ggAlgorithm.h"
#include "ggDebug.h"


ggController::ggController()
: mMode(eModeOff),
  mSetPointValue(20.0f),
  mHysteresisValue(1.0f),
  mOutputAnalog(false),
  mInputValid(true),
  mInputMicrosLast(0),
  mInputValue(0.0f),
  mOutputValue(0.0f),
  mSamplePeriod(10.0f),
  mControlP(0.4f),
  mControlI(0.004f),
  mControlD(12.0f),
  mOutputMin(0.0f),
  mOutputMax(1.0f),
  mMicrosLast(0),
  mErrorLast(0.0f),
  mErrorOutputI(0.0f),
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
  if (ggIsFinite(aInput)) {
    unsigned long vMicros = micros();
    float vTimeDelta = (vMicros - mInputMicrosLast) / 1000000.0f;
    float vInputWeight = vTimeDelta / mSamplePeriod;
    vInputWeight = ggClamp(vInputWeight, 0.0f, 1.0f);
    mInputValue = (1.0f - vInputWeight) * mInputValue + vInputWeight * aInput;
    mInputMicrosLast = vMicros;
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
  mTickerPID.attach_ms(1000.0f * mSamplePeriod + 0.5f, [&] () {
    ControlOutput();
  });
}


void ggController::PrintDebug(const String& aName) const
{
  ggDebug vDebug("ggController", aName);
  vDebug.PrintF("mMode = %s\n", ToString(mMode.Get()).c_str());
  vDebug.PrintF("mInputValid = %d\n", mInputValid);
  vDebug.PrintF("mInputMicrosLast = %u\n", mInputMicrosLast);
  vDebug.PrintF("mInputValue = %f\n", mInputValue);
  vDebug.PrintF("mSetPointValue = %f\n", mSetPointValue.Get());
  vDebug.PrintF("mHysteresisValue = %f\n", mHysteresisValue.Get());
  vDebug.PrintF("mSamplePeriod = %f\n", mSamplePeriod);
  vDebug.PrintF("mPID = %f / %f / %f\n", mControlP.Get(), mControlI.Get(), mControlD.Get());
  vDebug.PrintF("mOutputAnalog = %d\n", mOutputAnalog.Get());
  vDebug.PrintF("mOutputValue = %f\n", mOutputValue);
  vDebug.PrintF("mOutputMin = %f\n", mOutputMin);
  vDebug.PrintF("mOutputMax = %f\n", mOutputMax);
  vDebug.PrintF("mMicrosLast = %u\n", mMicrosLast);
  vDebug.PrintF("mErrorLast = %f\n", mErrorLast);
  vDebug.PrintF("mErrorOutputI = %f\n", mErrorOutputI);
}


void ggController::ResetControlStatePID()
{
  mInputMicrosLast = micros();
  mMicrosLast = micros();
  mErrorLast = 0.0f;
  mErrorOutputI = 0.0f;
}


void ggController::ControlOutput()
{
  // output is "off" by default (safety)
  float vOutputValue = mOutputMin;
  switch (GetMode()) {
    case eModeOn: vOutputValue = mOutputMax; break;
    case eModeOff: vOutputValue = mOutputMin; break;
    case eModeOnBelow: if (mInputValid) ControlOutput(true, vOutputValue); break;
    case eModeOnAbove: if (mInputValid) ControlOutput(false, vOutputValue); break;
    case eModePID: if (mInputValid) ControlOutputPID(vOutputValue); break;
    default: vOutputValue = mOutputMin; break;
  }

  // "update" doesn't notify if output value unchanged
  UpdateOutput(vOutputValue);
}


void ggController::ControlOutput(bool aInverted, float& aOutput) const
{
  // without hysteresis, the analog-mode mutates into digital-mode (division by zero)
  if (GetOutputAnalog() && (GetHysteresis() != 0.0f)) {
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
    float vOutput = (mOutputMin + mOutputMax) / 2.0f;
    vOutput += vDifference / GetHysteresis() + 0.5f;
    aOutput = ggClamp<float>(vOutput, mOutputMin, mOutputMax);
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
    aOutput = aInverted ? mOutputMax : mOutputMin;
  }
  else if (mInputValue >= vValueMax) {
    // input is above hysteresis range
    aOutput = aInverted ? mOutputMin : mOutputMax;
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
  if (vTimeDelta <= 0.9f * mSamplePeriod) return;

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
    float vErrorI = (vError + mErrorLast) / 2.0f * vTimeDelta;
    mErrorOutputI += mControlI.Get() * vErrorI; // mid-point riemann sum
    // avoid windup of integral error, when output gets at its limits
    vOutput = ggClamp(vOutput, mOutputMin, mOutputMax);
    float vErrorOutputIMin = mOutputMin - vOutput;
    float vErrorOutputIMax = mOutputMax - vOutput;
    mErrorOutputI = ggClamp(mErrorOutputI, vErrorOutputIMin, vErrorOutputIMax);
    vOutput += mErrorOutputI;
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
