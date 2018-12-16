#include <Servo.h>


Servo mServoAlpha;
const int mAlphaMin = 10;
const int mAlphaMax = 180;

Servo mServoBeta;
const int mBetaMin = 10;
const int mBetaMax = 180;


const int mSensorAlphaPin = A3;
const int mSensorBetaPin = A2;


void setup()
{
  mServoAlpha.attach(9);
  mServoBeta.attach(8);
  pinMode(13, OUTPUT);
  Serial.begin(115200);
}


void DelayLED(int aDelay)
{
  digitalWrite(13, HIGH);
  delay(aDelay);
  digitalWrite(13, LOW);
}

/*
void TestServo()
{
  const int vDelay = 1000;
  mServoAlpha.write(mAlphaMin); mServoBeta.write(mBetaMin); DelayLED(vDelay);
  mServoAlpha.write(mAlphaMax); mServoBeta.write(mBetaMin); DelayLED(vDelay);
  mServoAlpha.write(mAlphaMax); mServoBeta.write(mBetaMax); DelayLED(vDelay);
  mServoAlpha.write(mAlphaMin); mServoBeta.write(mBetaMax); DelayLED(vDelay);
}


void TestSensor()
{
  int vSensorAlpha = analogRead(mSensorAlphaPin);
  int vSensorBeta = analogRead(mSensorBetaPin);
  Serial.print("a=");
  Serial.print(vSensorAlpha);
  Serial.print(" b=");
  Serial.print(vSensorBeta);
  Serial.println();
  delay(500);
}
*/

int Clamp(int aValue, int aMin, int aMax)
{
  if (aValue < aMin) return aMin;
  if (aValue > aMax) return aMax;
  return aValue;
}


int CalculateNewAngle(int aAngleCurrent,
                      int aAngleMin,
                      int aAngleMax,
                      int aSensorValue,
                      int aSensorTarget)
{
  int vSensorDelta = (aSensorTarget - aSensorValue) / 50;
  int vAngle = aAngleCurrent + Clamp(vSensorDelta, -5, 5);
  return Clamp(vAngle, aAngleMin, aAngleMax);
}  


void loop()
{
  // TestServo();
  // TestSensor();
  
  static int mAlpha = (mAlphaMax + mAlphaMin) / 2;
  static int mBeta = (mBetaMax + mBetaMin) / 2;
  
  int vSensorAlpha = analogRead(mSensorAlphaPin);
  int vSensorBeta = analogRead(mSensorBetaPin);

  mAlpha = CalculateNewAngle(mAlpha, mAlphaMin, mAlphaMax, vSensorAlpha, 400);
  mBeta = CalculateNewAngle(mBeta, mBetaMin, mBetaMax, vSensorBeta, 300);

  mServoAlpha.write(mAlpha);
  mServoBeta.write(mBeta);
  
  delay(50);
}

