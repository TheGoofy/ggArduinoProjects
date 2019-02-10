#include <Wire.h>
#include "ggQuadratureDecoder.h"

ggQuadratureDecoder mCounter(0x61);

void setup()
{
  Serial.begin(115200);
  mCounter.begin();
//  mCounter.Set(1000000);
//  mCounter.SetSpeedSamplingTime(10000);
//  mCounter.Disable();
//  delay(5000);
//  mCounter.Enable();
}

void loop()
{
  long vCounter = mCounter.Get();
  long vSpeed = mCounter.GetSpeed();
  
  long vCounterStop = 0;
  long vMicrosStop = 0;
  long vSamples = 100;

  long vMicrosStart = micros();
  
  for (int vCount = 0; vCount < vSamples; vCount++) {
    vCounterStop = mCounter.Get();
  }

  vMicrosStop = micros();
  
  long vCounterDelta = vCounterStop - vCounter;
  long vMicrosDelta = vMicrosStop - vMicrosStart;
  long vSpeed2 = (long long)vCounterDelta * 1000000 / vMicrosDelta;
  long vSamplesPerSecond = vSamples * 1000000 / vMicrosDelta;

  Serial.print("Count=");
  Serial.print(vCounter);
  Serial.print("  ");

  Serial.print("SpeedSampling=");
  Serial.print(mCounter.GetSpeedSamplingTime());
  Serial.print("  ");

  Serial.print("Speed=");
  Serial.print(vSpeed);
  Serial.print("  ");

  Serial.print("Speed2=");
  Serial.print(vSpeed2);
  Serial.print("  ");

  Serial.print("ReadsPerSec=");
  Serial.print(vSamplesPerSecond);
  Serial.print("  ");

  Serial.println();
  delay(200);
}

