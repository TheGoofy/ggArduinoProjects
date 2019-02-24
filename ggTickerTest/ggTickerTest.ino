#include "ggTicker.h"


ggTicker mTicker({1300, 100, 200, 100, 200, 100}, true);

const int mPin = 2;


void setup()
{
  Serial.begin(115200);
  Serial.println("");
  
  pinMode(mPin, OUTPUT);
  
  mTicker.OnTick([&] (int aIntervalIndex) {
    const bool vEven = (aIntervalIndex & 1) == 0;
    digitalWrite(mPin, vEven);
  });

  mTicker.Start();
}

void loop()
{
  mTicker.Run();
  yield();
}
