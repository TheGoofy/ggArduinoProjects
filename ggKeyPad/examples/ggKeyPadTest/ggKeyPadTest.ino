#include "ggKeyPadI2C.h"
#include "ggQuadratureDecoder.h"
#include "ggSampler.h"

ggKeyPadI2C mKeyPad(0x67);
ggQuadratureDecoder mCounter(0x67);
ggSampler mSampler(10.0f);

const long mValueMin = 0;
const long mValueMax = 200;
const long mValueCenter = (mValueMin + mValueMax) / 2;

long mMicrosLast = 0;

void setup()
{
  pinMode(13, OUTPUT);
  mKeyPad.begin();
  mCounter.begin();
  mCounter.Set(mValueCenter);
}

void loop()
{
  static long vValue = mValueCenter;

  long vMicros = micros();
  long vMicrosDelta = vMicros - mMicrosLast;
  if (vMicrosDelta >= mValueMax) {
    mMicrosLast = vMicros;
  }

  digitalWrite(13, vMicrosDelta < vValue);

  if (mSampler.IsDue()) {
    
    if (mKeyPad.SwitchingOn(ggKeyPadI2C::eButtonA)) mCounter.Set(mValueMin);
    if (mKeyPad.SwitchingOn(ggKeyPadI2C::eButtonB)) mCounter.Set(mValueCenter);
    if (mKeyPad.SwitchingOn(ggKeyPadI2C::eButtonC)) mCounter.Set(mValueMax);

    vValue = mCounter.Get();
    if (vValue < mValueMin) vValue = mValueMin;
    if (vValue > mValueMax) vValue = mValueMax;

    mKeyPad.SetLED(ggKeyPadI2C::eOrangeA, vValue <= mValueMin);
    mKeyPad.SetLED(ggKeyPadI2C::eBlueA,   vValue <= mValueCenter);
    mKeyPad.SetLED(ggKeyPadI2C::eOrangeC, vValue >= mValueCenter);
    mKeyPad.SetLED(ggKeyPadI2C::eBlueC,   vValue >= mValueMax);
  }
}
