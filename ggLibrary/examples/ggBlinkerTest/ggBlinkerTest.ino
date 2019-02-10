#include "ggButton.h"
#include "ggBlinker.h"

ggButton mButtonA(8, true, true);
ggButton mButtonB(9, true, true);

ggBlinker mBlinker(13);

void setup()
{
  mButtonA.begin();
  mButtonB.begin();
  mBlinker.begin();
  mBlinker.Blink(3);
}

void loop()
{
  mBlinker.run();
  if (mButtonA.SwitchingOn()) mBlinker.BlinkStart();
  if (mButtonB.SwitchingOn()) mBlinker.BlinkStop();
}

