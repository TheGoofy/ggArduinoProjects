#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

void AnalogWrite(int aChannel, int aValue)
{
  pwm.setPin(aChannel, aValue);
}

const int mButtonPin = A0;

void setup()
{
  Serial.begin(115200);
  Serial.println("ggTestPWM");
  pwm.begin();
  pwm.setOscillatorFrequency(27900000);  // The int.osc. is closer to 27MHz
  pwm.setPWMFreq(1600);  // 1600 is the maximum PWM frequency
  for (int vChannel = 0; vChannel < 16; vChannel++) {
    AnalogWrite(vChannel, vChannel * 273);
  }

  pinMode(3, FUNCTION_3);
  pinMode(3, INPUT_PULLUP);
  pinMode(mButtonPin, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
}

void loop()
{
  /*
  const int vCount = 10000;

  int vDummy = 0;

  unsigned long vStart = 0;
  unsigned long vStop = 0;

  vStart = micros();
  for (int i = 0; i < vCount; i++) {
    yield();
  }
  vStop = micros();
  unsigned long vYield = vStop - vStart;
  
  vStart = micros();
  for (int i = 0; i < vCount; i++) {
    vDummy = analogRead(A0);
    yield();
  }
  vStop = micros();
  unsigned long vAnalog = vStop - vStart;

  vStart = micros();
  for (int i = 0; i < vCount; i++) {
    vDummy = digitalRead(12);
    yield();
  }
  vStop = micros();
  unsigned long vDigital = vStop - vStart;

  Serial.printf("%d\t%d\t%d\t\n", vYield, vDigital, vAnalog);
  */
  Serial.printf("digitalread = %d\n", digitalRead(3));
  // Serial.printf("%d\n", analogRead(mButtonPin));
  delay(500);
  /*
  static const int vDelay = 500;
  for (int vOn = 1; vOn < 4096; vOn *= 2) {
    AnalogWrite(0, vOn);
    delay(vDelay);
  }
  */
}
