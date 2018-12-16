
void Delay(long aMicroSeconds)
{
  if (aMicroSeconds > 1000) delay(aMicroSeconds/1000);
  else delayMicroseconds(aMicroSeconds);
}

void Step(long aNumberOfSteps, long aDelayMicroSeconds = 100)
{
  for (long vStep = 0; vStep < aNumberOfSteps; vStep++) {
    digitalWrite(2, HIGH);
    Delay(aDelayMicroSeconds);
    digitalWrite(3, HIGH);
    Delay(aDelayMicroSeconds);
    digitalWrite(2, LOW);
    Delay(aDelayMicroSeconds);
    digitalWrite(3, LOW);
    Delay(aDelayMicroSeconds);
  }
}

void Blink(int aDelayMilliSeconds)
{
  digitalWrite(13, HIGH);
  delay(aDelayMilliSeconds);
  digitalWrite(13, LOW);
}

void setup()
{
  // switch off status LED
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  
  // switch off outputs
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
}

void loop()
{
  // total 50'000 x 4 steps
  // with increasing speed
  Step(    1, 500000); Blink(5);
  Step(    2, 200000); Blink(5);
  Step(    3, 100000); Blink(5);
  Step(    6,  50000); Blink(5);
  Step(   15,  20000); Blink(5);
  Step(   30,  10000); Blink(5);
  Step(   60,   5000); Blink(5);
  Step(  150,   2000); Blink(5);
  Step(  300,   1000); Blink(5);
  Step(  600,    500); Blink(5);
  Step( 1500,    200); Blink(5);
  Step( 3000,    100); Blink(5);
  Step( 6000,     50); Blink(5);
  Step(12000,     25); Blink(5);
  Step(26333,     15); Blink(5);

  // wait 1 second
  Blink(1000);
}
