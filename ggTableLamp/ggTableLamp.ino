

// pwm pins: 3, 5, 6, 9, 10, 11


void setup()
{
  pinMode(3, OUTPUT);
  analogWrite(3, 31);
  pinMode(5, OUTPUT);
  analogWrite(5, 127);
  pinMode(6, OUTPUT);
  analogWrite(6, 223);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(13, OUTPUT);
}


void loop()
{
  // initial values (255 = 3 * 5 * 17)
  static int vValue = 0;
  static int vInc = 17;

  // set outputs
  analogWrite(9, vValue);
  analogWrite(10, 255 - vValue);
  digitalWrite(13, !digitalRead(13));
  delay(250);

  // next iteration
  vValue += vInc;
  if (vValue > 255) { vValue = 255; vInc = -vInc; }
  if (vValue < 0) { vValue = 0; vInc = -vInc; }
}
