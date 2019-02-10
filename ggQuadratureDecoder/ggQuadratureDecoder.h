#ifndef _GG_QUADRATURE_DECODER_H__
#define _GG_QUADRATURE_DECODER_H__

#include <Arduino.h>
#include <Wire.h>

class ggQuadratureDecoder {

private:
  
  #define COMMAND_COUNTER             0x01
  #define COMMAND_SPEED               0x02
  #define COMMAND_SPEED_SAMPLING_TIME 0x03
  #define COMMAND_ENABLE              0x04
  
public:
  
  ggQuadratureDecoder(byte aAddressI2C = 0x60);
  
  void begin();
  
  long Get() const;
  void Set(long aCounter);
  long GetSpeed() const;
  long GetSpeedSamplingTime() const;
  void SetSpeedSamplingTime(long aMicroSeconds);
  void Enable();
  void Disable();
  
private:
  
  bool SetCommand(byte aCommand) const;

  template <typename TValue>
  bool Receive(byte aCommand, TValue& aValue) const {
    if (!SetCommand(aCommand)) Serial.println("Receive(...) - SetCommand failed");
    byte* vValueByte = (byte*)&aValue;
    Wire.requestFrom(mAddressI2C, sizeof(aValue));
    int vNumberOfBytes = Wire.available();
    if (vNumberOfBytes == sizeof(aValue)) {
      while (vNumberOfBytes--) *vValueByte++ = Wire.read();
      return true;
    }
    // Serial.print("Receive(...) - Wrong Number of Bytes. Expected:");
    // Serial.print(sizeof(aValue));
    // Serial.print(" Received:");
    // Serial.println(vNumberOfBytes);
    return false;
  }

  template <typename TValue>
  bool Send(byte aCommand, TValue aValue) const {
    Wire.beginTransmission(mAddressI2C);
    Wire.write(aCommand);
    Wire.write((byte*)(&aValue), sizeof(aValue));
    return Wire.endTransmission() == 0;
  }

  byte mAddressI2C;
  
  long mCounter;
  long mSpeed;
  
};

#endif // _GG_QUADRATURE_DECODER_H__
