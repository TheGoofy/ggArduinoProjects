/*
  ESP8266 Blink by Simon Peter
  Blink the blue LED on the ESP-01 module
  This example code is in the public domain

  The blue LED on the ESP-01 module is connected to GPIO1
  (which is also the TXD pin; so we cannot use Serial.print() at the same time)

  Note that this sketch uses LED_BUILTIN to find the pin with the internal LED
*/


#define LED_COUNT 4


const int mLed[LED_COUNT] = {
  0, // GP0
  1, // GP1, TX
  2, // GP2
  3  // GP3, RX
};


void SetLed(int aLed, bool aOn)
{
  digitalWrite(mLed[aLed], aOn);
}


void setup()
{
  for (int vLed = 0; vLed < LED_COUNT; vLed++) {
    pinMode(mLed[vLed], OUTPUT);
    SetLed(vLed, false);
  }
}


void loop()
{
  static int vLedOn = 0;
  
  for (int vLed = 0; vLed < LED_COUNT; vLed++) {
    SetLed(vLed, vLed == vLedOn);
  }
  
  delay(250);
  
  vLedOn++;
  if (vLedOn >= LED_COUNT) vLedOn = 0;
}

