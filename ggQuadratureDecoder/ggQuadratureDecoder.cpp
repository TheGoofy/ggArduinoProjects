#include "ggQuadratureDecoder.h"

#include <Wire.h>


ggQuadratureDecoder::ggQuadratureDecoder(byte aAddressI2C)
: mAddressI2C(aAddressI2C),
  mCounter(0),
  mSpeed(0)
{
}


void ggQuadratureDecoder::begin()
{
  Wire.begin();
}


long ggQuadratureDecoder::Get() const
{
  Receive(COMMAND_COUNTER, mCounter);
  return mCounter;
}


void ggQuadratureDecoder::Set(long aCounter)
{
  mCounter = aCounter;
  Send(COMMAND_COUNTER, mCounter);
}


long ggQuadratureDecoder::GetSpeed() const
{
  Receive(COMMAND_SPEED, mSpeed);
  return mSpeed;
}


long ggQuadratureDecoder::GetSpeedSamplingTime() const
{
  long vSpeedSamplingTime = 0;
  Receive(COMMAND_SPEED_SAMPLING_TIME, vSpeedSamplingTime);
  return vSpeedSamplingTime;
}


void ggQuadratureDecoder::SetSpeedSamplingTime(long aMicroSeconds)
{
  Send(COMMAND_SPEED_SAMPLING_TIME, aMicroSeconds);
}


void ggQuadratureDecoder::Enable()
{
  Send(COMMAND_ENABLE, true);
}


void ggQuadratureDecoder::Disable()
{
  Send(COMMAND_ENABLE, false);
}


bool ggQuadratureDecoder::SetCommand(byte aCommand) const
{
  Wire.beginTransmission(mAddressI2C);
  Wire.write(aCommand);
  return Wire.endTransmission() == 0;
}
