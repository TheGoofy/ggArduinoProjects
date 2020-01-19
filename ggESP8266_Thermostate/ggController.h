#pragma once

#include <functional>
#include <Stream.h>

#include "ggValueEEPromT.h"
#include "ggStringConvertNumbers.h"
#include "ggSampler.h"

class ggController {

public:

  ggController();
  virtual ~ggController();

  void ResetSettings();

  enum tMode {
    eModeOff = 0,
    eModeOn = 1,
    eModeOnBelow = 2,
    eModeOnAbove = 3,
    eModePID = 4
  };

  tMode GetMode() const;
  void SetMode(tMode aMode);
  float GetSetPoint() const;
  void SetSetPoint(float aSetPoint);
  float GetHysteresis() const;
  void SetHysteresis(float aHysteresis);
  bool GetOutputAnalog() const;
  void SetOutputAnalog(bool aOutputAnalog);

  bool GetInputValid() const;
  void SetInputValid(bool aValid);
  float GetInput() const;
  void SetInput(float aInput);
  float GetOutput() const;

  const float& GetP() const;
  const float& GetI() const;
  const float& GetD() const;
  void SetPID(float aP, float aI, float aD);

  typedef std::function<void(float aOutputValue)> tOutputChangedFunc;
  void OnOutputChanged(tOutputChangedFunc aOutputChangedFunc);

  void Begin();
  void Run();

  void PrintDebug(const String& aName = "") const;

private:

  // reset internal controller state (in particular integreal)
  void ResetControlStatePID();

  void ControlOutput();
  void ControlOutput(bool aInverted, float& aOutput) const;
  void ControlOutputAnalog(bool aInverted, float& aOutput) const;
  void ControlOutputDigital(bool aInverted, float& aOutput) const;
  void ControlOutputPID(float& aOutput) const;

  void UpdateOutput(float aOutput);

  // persistent settings
  ggValueEEPromT<tMode> mMode;
  ggValueEEPromT<float> mSetPointValue;
  ggValueEEPromT<float> mHysteresisValue;
  ggValueEEPromT<bool> mOutputAnalog;

  // variable controls
  bool mInputValid;
  unsigned long mInputMicrosLast;
  float mInputValue;
  float mOutputValue;

  // PID controller
  ggSampler mSamplerPID;
  ggValueEEPromT<float> mControlP;
  ggValueEEPromT<float> mControlI;
  ggValueEEPromT<float> mControlD;
  float mOutputMin;
  float mOutputMax;
  mutable unsigned long mMicrosLast;
  mutable float mErrorLast;
  mutable float mErrorOutputI;

  // callback when output value changed
  tOutputChangedFunc mOutputChangedFunc;
  
};

template <>
String ToString(const ggController::tMode& aMode);

template <>
ggController::tMode FromString(const String& aString);
