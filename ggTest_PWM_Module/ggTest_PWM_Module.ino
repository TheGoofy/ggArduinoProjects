#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

void AnalogWrite(int aChannel, int aValue)
{
  pwm.setPin(aChannel, aValue);
}

void setup()
{
  pwm.begin();
  pwm.setOscillatorFrequency(28000000);  // The int.osc. is closer to 27MHz
  pwm.setPWMFreq(1000);  // 1600 is the maximum PWM frequency
  for (int vChannel = 0; vChannel < 16; vChannel++) {
    AnalogWrite(vChannel, vChannel * 273);
  }
}

void loop()
{
  static const int vDelay = 500;
  for (int vOn = 1; vOn < 4096; vOn *= 2) {
    AnalogWrite(0, vOn);
    delay(vDelay);
  }
}
