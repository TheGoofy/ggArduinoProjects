#include "ggJoyStick.h"

ggJoyStick mJoyStick(A0, A1, A2);

void setup()
{
  Serial.begin(115200);
  mJoyStick.begin();
  mJoyStick.CalibrateX(480, 2, 1018);
  mJoyStick.CalibrateY(524, 2, 1019);
  mJoyStick.CalibrateSwitch(false);
}

void loop()
{
  Serial.print("x=");
  Serial.print(mJoyStick.GetX());
  Serial.print("  ");

  Serial.print("y=");
  Serial.print(mJoyStick.GetY());
  Serial.print("  ");

  Serial.print("s=");
  Serial.print(mJoyStick.GetSwitch());
  Serial.print("  ");

  Serial.println();
  delay(200);
}

