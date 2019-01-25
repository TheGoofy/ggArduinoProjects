#include "ggBlinker.h"


ggBlinker::ggBlinker(byte aPin,
                     boolean aInverted,
                     long aMillisOn,
                     long aMillisOff)
: mPin(aPin),
  mInverted(aInverted),
  mMillisStop(0),
  mMillisOn(aMillisOn),
  mMillisOff(aMillisOff),
  mCount(0)
{
}


void ggBlinker::begin()
{
  pinMode(mPin, OUTPUT);
  digitalWrite(mPin, mInverted);
}


void ggBlinker::Blink(long aCount)
{
  mCount = 2 * aCount - 1;
  mMillisStop = millis() + mMillisOn;
  digitalWrite(mPin, !mInverted);
}


void ggBlinker::BlinkStart()
{
  Blink(0);
}


void ggBlinker::BlinkStop()
{
  mCount = 0;
  digitalWrite(mPin, mInverted);
}


void ggBlinker::BlinkSeconds(float aSeconds)
{
  long vCount = 1000.0f * aSeconds / (mMillisOn + mMillisOff);
  if (vCount == 0) vCount = 1;
  Blink(vCount);
}


bool ggBlinker::IsBlinking() const
{
  return mCount != 0;
}


void ggBlinker::SetMillisOn(long aMillisOn)
{
  mMillisOn = aMillisOn;
}


void ggBlinker::SetMillisOff(long aMillisOff)
{
  mMillisOff = aMillisOff;
}


long ggBlinker::GetMillisOn() const
{
  return mMillisOn;
}


long ggBlinker::GetMillisOff() const
{
  return mMillisOff;
}


void ggBlinker::run()
{
  if (mCount != 0) {
    long vMillis = millis();
    if (vMillis >= mMillisStop) {
      boolean mOff = mCount & 1;
      digitalWrite(mPin, !mInverted ^ mOff);
      mMillisStop = vMillis + (mOff ? mMillisOff : mMillisOn);
      if (mCount == -2) mCount = 0;
      mCount--;
    }
  }
}
