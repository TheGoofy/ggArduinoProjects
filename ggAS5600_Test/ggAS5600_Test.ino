#include <i2c_t3.h> // teensy3

#include "ggAS5600.h"


ggAS5600 mAS5600(0x36);


void setup()
{
  Serial.begin(38400);
  while (!Serial); // wait until USB serial ready
  Serial.println("AS5600 Test!");
  Serial.flush();
  
  Wire.begin(I2C_MASTER, 0, I2C_PINS_18_19, I2C_PULLUP_INT, I2C_RATE_1000);

  bool vSuccess = mAS5600.Begin();
  if (!vSuccess) Serial.print("AS5600 begin failed!\n");

  Serial.println("Registers:");
  mAS5600.Print(Serial);

  if (!mAS5600.ReadStatusMagnetDetected()) Serial.printf("Magnet missing!\n");
  if (mAS5600.ReadStatusMagnetTooWeak()) Serial.printf("Magnet too weak!\n");
  if (mAS5600.ReadStatusMagnetTooStrong()) Serial.printf("Magnet too strong!\n");

  // set current angle as zero
  mAS5600.WriteZPOS(mAS5600.ReadRawAngle());
}


void loop()
{
  delayMicroseconds(500000);

  unsigned long vMicrosStart = micros();

  // read and wait for angle from sensor (sync request)
  // uint16_t vAngle = mAS5600.ReadAngle();

  // get (last) angle and trigger new read from sensor (async request)
  bool vAngleRequestPending = mAS5600.AngleRequestPending();
  uint16_t vAngle = mAS5600.GetAngle();
  bool vAngleRequestSent = mAS5600.SendAngleRequest();

  unsigned long vMicrosDelta = micros() - vMicrosStart;

  // Serial.printf("%d\n", mAS5600.ReadMagnitude());
  // Serial.printf("%d\n", vAngle);
  Serial.printf("time: %d us, pend: %d, angle: %d, sent: %d\n", vMicrosDelta, vAngleRequestPending, vAngle, vAngleRequestSent);
}
