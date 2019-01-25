#include <Wire.h>
#include "ggQuadratureDecoder.h"
#include "ggKeyPadI2C_Slave.h"
#include "ggButtonSimple.h"

#define I2C_SLAVE_ADDRESS 0x67

#define LED_PIN_A0 A3 // left orange LED
#define LED_PIN_A1 A2 // left blue LED
#define LED_PIN_B LED_BUILTIN // onboard LED (for debugging)
#define LED_PIN_C0 11 // right orange LED
#define LED_PIN_C1 10 // right blue LED

#define BUTTON_PIN_A 5 // left button
#define BUTTON_PIN_B0 4 // decoder button (center)
#define BUTTON_PIN_B1 2 // decoder pin A
#define BUTTON_PIN_B2 3 // decoder pin B
#define BUTTON_PIN_C 6 // right button

ggQuadratureDecoder mDecoder;

ggButtonSimple mButtonA(BUTTON_PIN_A, true, true);
ggButtonSimple mButtonB(BUTTON_PIN_B0, true, true);
ggButtonSimple mButtonC(BUTTON_PIN_C, true, true);

#define COMMAND_COUNTER             0x01
#define COMMAND_SPEED               0x02
#define COMMAND_SPEED_SAMPLING_TIME 0x03
#define COMMAND_ENABLE              0x04

#define COMMAND_BUTTON_A            0x10
#define COMMAND_BUTTON_B            0x11
#define COMMAND_BUTTON_C            0x12

#define COMMAND_LED_A0_ON           0x20
#define COMMAND_LED_A0_OFF          0x21
#define COMMAND_LED_A1_ON           0x22
#define COMMAND_LED_A1_OFF          0x23
#define COMMAND_LED_C0_ON           0x24
#define COMMAND_LED_C0_OFF          0x25
#define COMMAND_LED_C1_ON           0x26
#define COMMAND_LED_C1_OFF          0x27

byte mCommand = COMMAND_COUNTER;

void RequestEvent() 
{
  // Serial.print("RequestEvent(): mCommand = ");
  // Serial.println(mCommand);
  switch (mCommand) {
    case COMMAND_COUNTER: Request(mDecoder.getCounter()); break;
    case COMMAND_SPEED: Request(mDecoder.getSpeed()); break;
    case COMMAND_SPEED_SAMPLING_TIME: Request(mDecoder.getSpeedSamplingTime()); break;
    case COMMAND_ENABLE: Request(mDecoder.getEnable()); break;
    case COMMAND_BUTTON_A: Request(mButtonA.GetOn()); break;
    case COMMAND_BUTTON_B: Request(mButtonB.GetOn()); break;
    case COMMAND_BUTTON_C: Request(mButtonC.GetOn()); break;
    default: break;
  }
}

void ReceiveEvent(int aNumberOfBytes)
{
  // get the number of bytes
  int vNumberOfBytes = Wire.available();
  // if we receive 1 byte, it's the value of a command
  if (aNumberOfBytes == 1) {
    mCommand = Wire.read();
    switch (mCommand) {
      case COMMAND_LED_A0_ON: digitalWrite(LED_PIN_A0, HIGH); break;
      case COMMAND_LED_A0_OFF: digitalWrite(LED_PIN_A0, LOW); break;
      case COMMAND_LED_A1_ON: digitalWrite(LED_PIN_A1, HIGH); break;
      case COMMAND_LED_A1_OFF: digitalWrite(LED_PIN_A1, LOW); break;
      case COMMAND_LED_C0_ON: digitalWrite(LED_PIN_C0, HIGH); break;
      case COMMAND_LED_C0_OFF: digitalWrite(LED_PIN_C0, LOW); break;
      case COMMAND_LED_C1_ON: digitalWrite(LED_PIN_C1, HIGH); break;
      case COMMAND_LED_C1_OFF: digitalWrite(LED_PIN_C1, LOW); break;
      default: break;
    }
  }
  // if we receive more bytes, it's the command with its value(s)
  if (aNumberOfBytes > 1) {
    long vValueLong = 0;
    bool vValueBool = false;
    mCommand = Wire.read();
    switch (mCommand) {
      case COMMAND_COUNTER: Receive(vValueLong); mDecoder.setCounter(vValueLong); break;
      case COMMAND_SPEED: /* Receive(mSpeed); */ break; // speed is read-only
      case COMMAND_SPEED_SAMPLING_TIME: Receive(vValueLong); mDecoder.setSpeedSamplingTime(vValueLong); break;
      case COMMAND_ENABLE: Receive(vValueBool); mDecoder.enable(vValueBool); break;
      default: break;
    }
  }
  // read all remaining bytes, and ignore them
  while (Wire.available()) Wire.read();
}

void FlashLEDs(int aDelay = 300/8)
{
  digitalWrite(LED_PIN_A0, HIGH); delay(aDelay);
  digitalWrite(LED_PIN_A1, HIGH); delay(aDelay);
  digitalWrite(LED_PIN_A0, LOW); delay(aDelay);
  digitalWrite(LED_PIN_A1, LOW); delay(aDelay);
  digitalWrite(LED_PIN_C0, HIGH); delay(aDelay);
  digitalWrite(LED_PIN_C1, HIGH); delay(aDelay);
  digitalWrite(LED_PIN_C0, LOW); delay(aDelay);
  digitalWrite(LED_PIN_C1, LOW); delay(aDelay);
}

void setup()
{
  // Serial.begin(115200);
  // Serial.println("hello goofy");
  Wire.begin(I2C_SLAVE_ADDRESS);
  Wire.onRequest(RequestEvent);
  Wire.onReceive(ReceiveEvent);
  pinMode(LED_PIN_A0, OUTPUT);
  pinMode(LED_PIN_A1, OUTPUT);
  pinMode(LED_PIN_C0, OUTPUT);
  pinMode(LED_PIN_C1, OUTPUT);
  FlashLEDs();
  mDecoder.begin(BUTTON_PIN_B1, BUTTON_PIN_B2);
  mButtonA.begin();
  mButtonB.begin();
  mButtonC.begin();
}

void loop()
{
  mDecoder.run();
  mButtonA.Switching();
  mButtonB.Switching();
  mButtonC.Switching();
}

