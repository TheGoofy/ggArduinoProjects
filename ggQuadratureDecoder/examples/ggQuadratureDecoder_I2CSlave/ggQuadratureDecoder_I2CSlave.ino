// transform an arduino into something similar like an HCTL-2032

#include <Wire.h>

#define SLAVE_ADDRESS_BASE 0x60

#define SLAVE_ADDRESS_BIT_0_PIN 4
#define SLAVE_ADDRESS_BIT_1_PIN 5
#define SLAVE_ADDRESS_BIT_2_PIN 6

#define ENCODED_VALUE_A_PIN 2
#define ENCODED_VALUE_B_PIN 3

#define COMMAND_COUNTER             0x01
#define COMMAND_SPEED               0x02
#define COMMAND_SPEED_SAMPLING_TIME 0x03
#define COMMAND_ENABLE              0x04

#define STATUS_LED_PIN 13

boolean mEncodedValueA = false;
boolean mEncodedValueB = false;

byte mCommand = COMMAND_COUNTER;
boolean mEnable = true;
long mCounter = 0;
long mSpeed = 0;
long mSpeedSamplingTime = 100000; // 100'000 microseconds => 10 samples per second

void RequestEvent() 
{
  switch (mCommand) {
    case COMMAND_COUNTER: Wire.write((byte*)(&mCounter), sizeof(mCounter)); break;
    case COMMAND_SPEED: Wire.write((byte*)(&mSpeed), sizeof(mSpeed)); break;
    case COMMAND_SPEED_SAMPLING_TIME: Wire.write((byte*)(&mSpeedSamplingTime), sizeof(mSpeedSamplingTime)); break;
    case COMMAND_ENABLE: Wire.write((byte*)(&mEnable), sizeof(mEnable)); break;
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
  }
  // if we receive 5 bytes, it's the command with its value
  if (aNumberOfBytes > 1) {
    mCommand = Wire.read();
    switch (mCommand) {
      case COMMAND_COUNTER: Receive(mCounter); break;
      case COMMAND_SPEED: /* Receive(mSpeed); */ break; // speed is read-only
      case COMMAND_SPEED_SAMPLING_TIME: Receive(mSpeedSamplingTime); break;
      case COMMAND_ENABLE: Receive(mEnable); UpdateEnable(); break;
      default: break;
    }
  }
  // read all remaining bytes, and ignore them
  while (Wire.available()) Wire.read();
}

void Receive(boolean& aValue)
{
  if (Wire.available() >= sizeof(aValue)) {
    byte* vValueByte = (byte*)&aValue;
    byte* vValueByteEnd = (byte*)(&aValue + sizeof(aValue));
    noInterrupts();
    while (vValueByte != vValueByteEnd) *vValueByte++ = Wire.read();
    interrupts();
  }
}

void Receive(long& aValue)
{
  if (Wire.available() >= sizeof(aValue)) {
    byte* vValueByte = (byte*)&aValue;
    byte* vValueByteEnd = (byte*)(&aValue + sizeof(aValue));
    noInterrupts();
    while (vValueByte != vValueByteEnd) *vValueByte++ = Wire.read();
    interrupts();
  }
}

void UpdateEnable()
{
  if (mEnable) {
    attachInterrupt(0, ChangeEncodedValueA, CHANGE);
    attachInterrupt(1, ChangeEncodedValueB, CHANGE);
  }
  else {
    detachInterrupt(0);
    detachInterrupt(1);
  }
}

void ChangeEncodedValueA()
{
  if (mEncodedValueA != digitalRead(ENCODED_VALUE_A_PIN)) {
    mEncodedValueA = !mEncodedValueA;
    mCounter += mEncodedValueA ^ mEncodedValueB ? -1 : 1;
  }
}

void ChangeEncodedValueB()
{
  if (mEncodedValueB != digitalRead(ENCODED_VALUE_B_PIN)) {
    mEncodedValueB = !mEncodedValueB;
    mCounter += mEncodedValueA ^ mEncodedValueB ? 1 : -1;
  }
}

void setup()
{
  // Serial.begin(115200);
  
  pinMode(SLAVE_ADDRESS_BIT_0_PIN, INPUT);
  pinMode(SLAVE_ADDRESS_BIT_1_PIN, INPUT);
  pinMode(SLAVE_ADDRESS_BIT_2_PIN, INPUT);
  
  byte vSlaveAddress = SLAVE_ADDRESS_BASE;
  vSlaveAddress += digitalRead(SLAVE_ADDRESS_BIT_0_PIN) << 0;
  vSlaveAddress += digitalRead(SLAVE_ADDRESS_BIT_1_PIN) << 1;
  vSlaveAddress += digitalRead(SLAVE_ADDRESS_BIT_2_PIN) << 2;

  Wire.begin(vSlaveAddress);
  Wire.onRequest(RequestEvent);
  Wire.onReceive(ReceiveEvent);

  pinMode(ENCODED_VALUE_A_PIN, INPUT);
  pinMode(ENCODED_VALUE_B_PIN, INPUT);

  attachInterrupt(0, ChangeEncodedValueA, CHANGE);
  attachInterrupt(1, ChangeEncodedValueB, CHANGE);

  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);
}

void loop()
{
  // get time
  long vMicros = micros();

  // speed calculation
  if (mEnable) {
    static long vSpeedLastMicros = 0;
    static long vSpeedLastCounter = 0;
    if (vMicros - vSpeedLastMicros >= mSpeedSamplingTime) {
      long vCounter = mCounter;
      mSpeed = vCounter - vSpeedLastCounter;
      vSpeedLastMicros = vMicros;
      vSpeedLastCounter = vCounter;
    }
  }

  // LED motion indicator, if enabled
  if (mEnable) {
    static long vMotionLastMicros = 0;
    static long vMotionLastCounter = 0;
    if (vMicros - vMotionLastMicros >= 1000) {
      long vCounter = mCounter;
      digitalWrite(STATUS_LED_PIN, mCounter - vMotionLastCounter != 0);
      vMotionLastCounter = vCounter;
      vMotionLastMicros = vMicros;
    }
  }

  // blink if disabled
  if (!mEnable) {
    static long mBlinkLastMicros = 0;
    static boolean vBlink = true;
    if (vMicros - mBlinkLastMicros >= 500000) {
      digitalWrite(STATUS_LED_PIN, vBlink);
      mBlinkLastMicros = vMicros;
      vBlink = !vBlink;
    }
  }
  

  /*
  // for debugging
  static long vLastMicros1 = 0;
  if (vMicros - vLastMicros1 >= 500000L) {
    vLastMicros1 = vMicros;
    Serial.print("command=");
    Serial.print(mCommand);
    Serial.print(" enable=");
    Serial.print(mEnable);
    Serial.print(" speedsamplingtime=");
    Serial.print(mSpeedSamplingTime);
    Serial.print(" / counter=");
    Serial.print(mCounter);
    Serial.print(" speed=");
    Serial.println(mSpeed);
  }
  */
}

