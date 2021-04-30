#pragma once

class ggAS600L
{
public:

  ggAS600L(uint8_t aSlaveAddr = 0x36)
  : mSlaveAddr(aSlaveAddr & 0xf7),
    mRegAddr(0xFF)
  {
  }

  inline bool Reset() {
    mAngleRequest = false;
    mAngle = 0;
    mRegAddr = 0xFF;
    return TransmitRegAddr(0x00);
  }

  inline bool Begin() {
    Wire.onReqFromDone([] () {
      if (mAngleRequest) {
        mAngle = Wire.read() << 8 | Wire.read();
        mAngleRequest = false;
      }
    });
    return CheckSlave();
  }

  inline uint8_t GetSlaveAddr() const {
    return mSlaveAddr;
  }

  inline bool CheckSlave() const {
    BeginTransmission();
    return EndTransmission();
  }

  inline bool SendAngleRequest() {
    if (TransmitRegAddr(0x0E)) {
      mAngleRequest = true;
      Wire.sendRequest(mSlaveAddr, 2);
      return true;
    }
    return false;
  }

  inline bool ReceivedAngle() const {
    return !mAngleRequest;
  }

  inline uint16_t GetAngle() const {
    return mAngle;
  }

  inline uint16_t ReadAngle() const {
    mAngleRequest = false;
    Read(0x0E, mAngle);
    return mAngle;
  }
/*
  template <typename TSerial>
  void PrintRegister(TSerial& aSerial, uint8_t aRegAddr, const char* aName = nullptr) const {
    Serial.printf("[0x%02x] = 0x%02x <%s>\n", aRegAddr, ReadT<uint8_t>(aRegAddr), aName);
  }

  template <typename TSerial>
  void PrintRegisters(TSerial& aSerial) const {
    PrintRegister(aSerial, 0x00);
    PrintRegister(aSerial, 0x01);
    PrintRegister(aSerial, 0x02);
    PrintRegister(aSerial, 0x03);
    PrintRegister(aSerial, 0x04);
    PrintRegister(aSerial, 0x05);
    PrintRegister(aSerial, 0x06);
    PrintRegister(aSerial, 0x07);
    PrintRegister(aSerial, 0x08);
    PrintRegister(aSerial, 0x0C, "RAW ANGLE");
    PrintRegister(aSerial, 0x0D, "RAW ANGLE");
    PrintRegister(aSerial, 0x0E, "ANGLE");
    PrintRegister(aSerial, 0x0F, "ANGLE");
    PrintRegister(aSerial, 0x0B, "STATUS");
    PrintRegister(aSerial, 0x1A, "AGC");
    PrintRegister(aSerial, 0x1B, "MAGNITUDE");
    PrintRegister(aSerial, 0x1C, "MAGNITUDE");
    PrintRegister(aSerial, 0x20, "I2CADDR");
    PrintRegister(aSerial, 0x21, "I2CSTRB");
    Serial.println(ReadT<uint8_t>(0x0B), BIN);
  }
*/
private:

  inline void BeginTransmission() const {
    Wire.beginTransmission(mSlaveAddr);
  }
  
  inline void BeginTransmission(uint8_t aRegAddr) const {
    Wire.beginTransmission(mSlaveAddr);
    Wire.write(aRegAddr);
    mRegAddr = aRegAddr;
  }

  inline bool EndTransmission() const {
    return Wire.endTransmission() == 0;
  }

  inline bool TransmitRegAddr(uint8_t aRegAddr) const {
    if (mRegAddr != aRegAddr) {
      BeginTransmission(aRegAddr);
      return EndTransmission();
    }
    return true;
  }

  inline bool Read(uint8_t aRegAddr, uint8_t& aData) const {
    if (TransmitRegAddr(aRegAddr)) {
      uint8_t vBytesReceived = Wire.requestFrom(mSlaveAddr, (uint8_t)1);
      if (vBytesReceived != 1) {
        Serial.print("Expected 1 byte, but received ");
        Serial.print(vBytesReceived);
        Serial.print("!\n");
        return false;
      }
      else {
        aData = Wire.read();
        return true;
      }
    }
    return false;
  }

  inline bool Read(uint8_t aRegAddr, uint16_t& aData) const {
    if (TransmitRegAddr(aRegAddr)) {
      uint8_t vBytesReceived = Wire.requestFrom(mSlaveAddr, (uint8_t)2);
      if (vBytesReceived != 2) {
        Serial.print("Expected 2 bytes, but received ");
        Serial.print(vBytesReceived);
        Serial.print("!\n");
        return false;
      }
      else {
        aData = Wire.read() << 8 | Wire.read();
        return true;
      }
    }
    return false;
  }

  uint8_t mSlaveAddr;
  mutable uint8_t mRegAddr;

  static bool mAngleRequest;
  static uint16_t mAngle;

};


bool ggAS600L::mAngleRequest = false;
uint16_t ggAS600L::mAngle = 0;
