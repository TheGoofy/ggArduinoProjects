#pragma once

class ggFRAMI2C
{
public:

  // common slave-addr-range: 0x50..0x57
  ggFRAMI2C(uint8_t aSlaveAddr = 0x50)
  : mSlaveAddr(aSlaveAddr & 0xf7) {
  }

  inline bool Begin() {
    return CheckSlave();
  }

  inline uint8_t GetSlaveAddr() const {
    return mSlaveAddr;
  }

  inline bool CheckSlave() const {
    BeginTransmission();
    return EndTransmission();
  }

  inline bool Write(uint16_t aAddr, uint8_t aData) {
    BeginTransmission(aAddr);
    Wire.write(aData);
    return EndTransmission();
  }

  inline bool Write(uint16_t aAddr, const uint8_t* aData, uint16_t aDataSize) {
    BeginTransmission(aAddr);
    while (aDataSize--) Wire.write(*(aData++));
    return EndTransmission();
  }

  template <typename TData>
  bool WriteT(uint16_t aAddr, const TData& aData) {
    return Write(aAddr, (uint8_t*)&aData, sizeof(TData));
  }

  inline uint8_t Read(uint16_t aAddr) const {
    uint8_t vData = 0;
    Read(aAddr, vData);
    return vData;
  }

  inline bool Read(uint16_t aAddr, uint8_t& aData) const {
    BeginTransmission(aAddr);
    if (EndTransmission()) {
      uint8_t vBytesReceived = Wire.requestFrom(mSlaveAddr, 1);
      if (vBytesReceived != 1) {
        Serial.print("Expected 1 byte, but received ");
        Serial.print(vBytesReceived);
        Serial.print("!\n");
        return false;
      }
      else {
        aData = Wire.read();
      }
    }
    return false;
  }

  inline bool Read(uint16_t aAddr, uint8_t* aData, uint16_t aDataSize) const {
    BeginTransmission(aAddr);
    if (EndTransmission()) {
      uint8_t vBytesReceived = Wire.requestFrom(mSlaveAddr, aDataSize);
      if (vBytesReceived != aDataSize) {
        Serial.print("Expected ");
        Serial.print(aDataSize);
        Serial.print(" bytes, but received ");
        Serial.print(vBytesReceived);
        Serial.print("!\n");
        return false;
      }
      else {
        while (Wire.available()) *(aData++) = Wire.read();
        return true;
      }
    }
    return false;
  }
  
  template <typename TData>
  inline bool ReadT(uint16_t aAddr, TData& aData) const {
    return Read(aAddr, (uint8_t*)&aData, sizeof(TData));
  }

  template <typename TData>
  inline TData ReadT(uint16_t aAddr) const {
    TData vData;
    ReadT(aAddr, vData);
    return vData;
  }

private:

  enum tI2CResult {
    eI2CResultOK = 0,
    eI2CResultErrorDataTooLong = 1,
    eI2CResultErrorNackOnAddr = 2,
    eI2CResultErrorNackOnData = 3,
    eI2CResultErrorOther = 4
  };

  inline void BeginTransmission() const {
    Wire.beginTransmission(mSlaveAddr);
  }
  
  inline void BeginTransmission(uint16_t aAddr) const {
    Wire.beginTransmission(mSlaveAddr);
    Wire.write(aAddr >> 8);
    Wire.write(aAddr & 0xff);
  }

  inline bool EndTransmission() const {
    tI2CResult vResult = static_cast<tI2CResult>(Wire.endTransmission());
    PrintResultError(vResult);
    return vResult == eI2CResultOK;
  }

  void PrintResultError(tI2CResult aI2CResult) const {
    switch (aI2CResult) {
      case eI2CResultOK: return;
      case eI2CResultErrorDataTooLong: Serial.println("I2C error: data too long");
      case eI2CResultErrorNackOnAddr: Serial.println("I2C error: nack on addr");
      case eI2CResultErrorNackOnData: Serial.println("I2C error: nack on data");
      case eI2CResultErrorOther: Serial.println("I2C error: other");
    }
  }

  uint8_t mSlaveAddr;
  
};
