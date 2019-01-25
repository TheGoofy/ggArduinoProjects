#ifndef __GG_KEY_PAD__
#define __GG_KEY_PAD__

#include <Arduino.h>


template <typename T>
void SerialPrintType(T aValue) {Serial.print(sizeof(T)); }
void SerialPrintType(bool) { Serial.print("bool"); }
void SerialPrintType(char) { Serial.print("char"); }
void SerialPrintType(int) { Serial.print("int"); }
void SerialPrintType(long) { Serial.print("long"); }
void SerialPrintType(unsigned char) { Serial.print("unsigned char"); }
void SerialPrintType(unsigned int) { Serial.print("unsigned int"); }
void SerialPrintType(unsigned long) { Serial.print("unsigned long"); }
void SerialPrintType(float) { Serial.print("float"); }
void SerialPrintType(double) { Serial.print("double"); }


template <typename TValue>
void Request(TValue aValue) {
  // Serial.print("Request<");
  // SerialPrintType(aValue);
  // Serial.print(">(");
  // Serial.print(aValue);
  // Serial.println(")");
  Wire.write((byte*)(&aValue), sizeof(aValue));
}


template <typename TValue>
void Receive(TValue& aValue) {
  // Serial.print("Receive<");
  // SerialPrintType(aValue);
  // Serial.print(">(");
  if (Wire.available() >= sizeof(aValue)) {
    byte* vValueByte = (byte*)&aValue;
    byte* vValueByteEnd = vValueByte + sizeof(aValue);
    noInterrupts();
    while (vValueByte != vValueByteEnd) *vValueByte++ = Wire.read();
    interrupts();
  }
  // Serial.print(aValue);
  // Serial.println(")");
}

#endif // __GG_KEY_PAD__
