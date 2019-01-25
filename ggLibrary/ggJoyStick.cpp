#include "ggJoyStick.h"


ggJoyStick::ggJoyStick(int aPinX, int aPinY, int aPinSwitch)
{
  mPin[0] = aPinX;
  mPin[1] = aPinY;
  mPinSwitch = aPinSwitch;
  CalibrateX();
  CalibrateY();
  CalibrateSwitch(false);
}


void ggJoyStick::begin()
{
  pinMode(mPin[0], INPUT);
  pinMode(mPin[1], INPUT);
  pinMode(mPinSwitch, INPUT);
}


void ggJoyStick::CalibrateX(int aCenter, int aMin, int aMax)
{
  Calibrate(0, aCenter, aMin, aMax);
}


void ggJoyStick::CalibrateY(int aCenter, int aMin, int aMax)
{
  Calibrate(1, aCenter, aMin, aMax);
}


void ggJoyStick::CalibrateSwitch(boolean aInvert)
{
  mSwitchInvert = aInvert;
}


float ggJoyStick::GetX() const
{
  return GetValue(0);
}


float ggJoyStick::GetY() const
{
  return GetValue(1);
}


boolean ggJoyStick::GetSwitch() const
{
  return digitalRead(mPinSwitch) ^ !mSwitchInvert;
}
  

float ggJoyStick::GetValue(int aAxis) const
{
  int vValue = analogRead(mPin[aAxis]) - mCenter[aAxis];
  if (vValue < 0) return mScaleL[aAxis] * vValue;
  else return mScaleR[aAxis] * vValue;
}


void ggJoyStick::Calibrate(int aAxis, int aCenter, int aMin, int aMax)
{
  mCenter[aAxis] = aCenter;
  if (aMin < aMax) {
    mScaleL[aAxis] = 1.0f / (aCenter - aMin);
    mScaleR[aAxis] = 1.0f / (aMax - aCenter);
  }
  else {
    mScaleL[aAxis] = 1.0f / (aMax - aCenter);
    mScaleR[aAxis] = 1.0f / (aCenter - aMin);
  }
}
