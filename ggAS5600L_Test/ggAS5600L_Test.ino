#include <i2c_t3.h> // teensy3

#include "ggAS600L.h"


ggAS600L mAS600L(0x36);


void setup()
{
  Serial.begin(115200);
  Serial.println("\nAS5600 Test!");
  
  Wire.begin(I2C_MASTER, 0, I2C_PINS_18_19, I2C_PULLUP_INT, I2C_RATE_1000);

  bool vSuccess = mAS600L.Begin();
  if (!vSuccess) Serial.print("AS600L begin failed!\n");

  vSuccess = mAS600L.SendAngleRequest();
  if (!vSuccess) Serial.print("AS600L angle request failed!\n");  

  // mAS600L.PrintRegisters(Serial);
/*
  mAS600L.ReadAngle();
  
  Wire.onReqFromDone([] () {
    if (mAngleRequested) {
      mAngle = Wire.read() << 8 | Wire.read();
      mAngleRequested = false;
    }
  });
*/
}


void loop()
{
  delayMicroseconds(20000);

  unsigned long vMicrosStart = micros();

  // read and wait for angle from sensor (sync request)
  // uint16_t vAngle = mAS600L.ReadAngle();

  // get (last) angle and trigger new read from sensor (async request)
  bool vSuccess = mAS600L.ReceivedAngle();
  uint16_t vAngle = mAS600L.GetAngle();
  if (!mAS600L.SendAngleRequest()) Serial.print("Failed to send angle request!\n");

  unsigned long vMicrosDelta = micros() - vMicrosStart;

  if (!vSuccess) {
    mAS600L.Reset();
    //mAS600L.Begin();
  }

  // Serial.printf("time: %d us, status: %d, angle: %d\n", vMicrosDelta, vSuccess, vAngle);
  Serial.printf("%d\n", vAngle);

 
}
