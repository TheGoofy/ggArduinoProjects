#pragma once

// https://ams.com/documents/20143/36005/AS5600_DS000365_5-00.pdf

class ggAS5600
{
public:

  enum tRegAddr {
    eRegZMCO      = 0x00, // programmed      (read)
    eRegZPOS      = 0x01, // start position  (read / write / program)
    eRegMPOS      = 0x03, // stop position   (read / write / program)
    eRegMANG      = 0x05, // maximum angle   (read / write / program)
    eRegCONF      = 0x07, // PWM config      (read / write / program)
    eRegRawAngle  = 0x0C, // raw angle       (read)
    eRegAngle     = 0x0E, // angle           (read)
    eRegStatus    = 0x0B, // magnet statur   (read)
    eRegAGC       = 0x1A, // auto gain ctrl  (read)
    eRegMagnitude = 0x1B, // magnet strength (read)
    eRegBurn      = 0xFF, // prog ang/conf   (write)
    eRegInvalid   = 0xFE  // no register
  };

  ggAS5600(uint8_t aSlaveAddr = 0x36, int aDirPin = -1)
  : mSlaveAddr(aSlaveAddr),
    mDirPin(aDirPin),
    mRegAddr(eRegInvalid)
  {
  }

  inline bool Reset() {
    mAngleRequestPending = false;
    mAngle = 0;
    mRegAddr = eRegInvalid;
    return TransmitRegAddr(eRegAngle);
  }

  inline bool Begin() {
    Wire.onReqFromDone([] () {
      if (mAngleRequestPending) {
        mAngle = (Wire.read() << 8 | Wire.read()) & 0x0FFF;
        mAngleRequestPending = false;
      }
    });
    if (mDirPin != -1) pinMode(mDirPin, OUTPUT);
    return CheckSlave();
  }

  inline uint8_t GetSlaveAddr() const {
    return mSlaveAddr;
  }

  inline bool CheckSlave() const {
    BeginTransmission();
    return EndTransmission();
  }

  void SetDirCW() {
    if (mDirPin != -1) digitalWrite(mDirPin, LOW);
  }

  void SetDirCCW() {
    if (mDirPin != -1) digitalWrite(mDirPin, HIGH);
  }

  inline bool SendAngleRequest() {
    if (TransmitRegAddr(eRegAngle)) {
      mAngleRequestPending = true;
      Wire.sendRequest(mSlaveAddr, 2);
      return true;
    }
    return false;
  }

  inline bool AngleRequestPending() const {
    return mAngleRequestPending;
  }

  inline uint16_t GetAngle() const {
    return mAngle;
  }

  inline uint8_t ReadZMCO() const {
    return ReadT<uint8_t>(eRegZMCO) & 0x03F;
  }

  inline uint16_t ReadZPOS() const {
    return ReadT<uint16_t>(eRegZPOS) & 0x0FFF;
  }

  inline bool WriteZPOS(uint16_t aZPOS) {
    return Write(eRegZPOS, aZPOS);
  }

  inline uint16_t ReadMPOS() const {
    return ReadT<uint16_t>(eRegMPOS) & 0x0FFF;
  }

  inline bool WriteMPOS(uint16_t aMPOS) {
    return Write(eRegMPOS, aMPOS);
  }

  inline uint16_t ReadMANG() const {
    return ReadT<uint16_t>(eRegMANG) & 0x0FFF;
  }

  inline bool WriteMANG(uint16_t aMANG) {
    return Write(eRegMANG, aMANG);
  }

  inline uint16_t ReadCONF() const {
    return ReadT<uint16_t>(eRegCONF) & 0x3FFF;
  }

  inline bool WriteCONF(uint16_t aCONF) {
    return Write(eRegCONF, aCONF);
  }

  enum tSlowFilter {
    eSlowFilterMask = 0x0300, // 0000'00XX'0000'0000 bits 9:8
    eSlowFilter16x  = 0x0000, //        00           response time: 2.2 ms (low noise)
    eSlowFilter08x  = 0x0100, //        01           response time: 1.1 ms
    eSlowFilter04x  = 0x0200, //        10           response time: 0.55 ms
    eSlowFilter02x  = 0x0300  //        11           response time: 0.286 ms (high noise)
  };

  inline tSlowFilter ReadCONFSlowFilter() const {
    return ReadCONF() & eSlowFilterMask;
  }

  inline bool WriteCONFSlowFilter(tSlowFilter aSlowFilter) {
    return WriteCONF((ReadCONF() & ~eSlowFilterMask) | aSlowFilter);
  }

  enum tFastFilterThreshold {
    eFastFilterThresholdMask   = 0x1C00, // 000X'XX00'0000'0000 bits 12:10
    eFastFilterThresholdSFOnly = 0x0000, //    0'00
    eFastFilterThreshold6LSB   = 0x0400, //    0'01
    eFastFilterThreshold7LSB   = 0x0800, //    0'10
    eFastFilterThreshold9LSB   = 0x0C00, //    0'11
    eFastFilterThreshold18LSB  = 0x1000, //    1'00
    eFastFilterThreshold21LSB  = 0x1400, //    1'01
    eFastFilterThreshold24LSB  = 0x1800, //    1'10
    eFastFilterThreshold10LSB  = 0x1C00  //    1'11
  };

  inline tFastFilterThreshold ReadCONFFastFilterThreshold() const {
    return ReadCONF() & eFastFilterThresholdMask;
  }

  inline bool WriteCONFFastFilterThreshold(tFastFilterThreshold aFastFilterThreshold) {
    return WriteCONF((ReadCONF() & ~eFastFilterThresholdMask) | aFastFilterThreshold);
  }

  inline uint16_t ReadRawAngle() const {
    return ReadT<uint16_t>(eRegRawAngle) & 0x0FFF;
  }

  inline uint16_t ReadAngle() const {
    return ReadT<uint16_t>(eRegAngle) & 0x0FFF;
  }

  enum tStatusMagnet {
    eStatusMask = 0x38, // 00XX'X000 bits 3:5
    eStatusMH   = 0x08, //   00'1    magnet high
    eStatusML   = 0x10, //   01'0    magnet low
    eStatusMD   = 0x20, //   10'0    magnet detect
  };

  inline uint8_t ReadStatus() const {
    return ReadT<uint8_t>(eRegStatus);
  }

  inline bool ReadStatusMagnetTooStrong() const {
    return ReadStatus() & eStatusMH; // MH bit 3
  }

  inline bool ReadStatusMagnetTooWeak() const {
    return ReadStatus() & eStatusML; // ML bit 4
  }

  inline bool ReadStatusMagnetDetected() const {
    return ReadStatus() & eStatusMD; // MD bit 5
  }

  inline uint8_t ReadAGC() const {
    return ReadT<uint8_t>(eRegAGC);
  }

  inline uint16_t ReadMagnitude() const {
    return ReadT<uint16_t>(eRegMagnitude);
  }

  template <typename TSerial>
  void Print(TSerial& aSerial) const {
    aSerial.printf("[0x%02X] ZMCO: %d\n", eRegZMCO, ReadZMCO());
    aSerial.printf("[0x%02X] ZPOS: %d\n", eRegZPOS, ReadZPOS());
    aSerial.printf("[0x%02X] MPOS: %d\n", eRegMPOS, ReadMPOS());
    aSerial.printf("[0x%02X] MANG: %d\n", eRegMANG, ReadMANG());
    aSerial.printf("[0x%02X] CONF: 0x%04x\n", eRegCONF, ReadCONF());
    aSerial.printf("[0x%02X] RawAngle: %d\n", eRegRawAngle, ReadRawAngle());
    aSerial.printf("[0x%02X] Angle: %d\n", eRegAngle, ReadAngle());
    aSerial.printf("[0x%02X] Status: 0x%02x (MD:%d ML:%d MH:%d)\n", eRegStatus, ReadStatus(), ReadStatusMagnetDetected(), ReadStatusMagnetTooWeak(), ReadStatusMagnetTooStrong());
    aSerial.printf("[0x%02X] AGC: %d\n", eRegAGC, ReadAGC());
    aSerial.printf("[0x%02X] Magnitude: %d\n", eRegMagnitude, ReadMagnitude());
  }

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
    // "angle", "raw-angle", and "magnitude" register addresses are not automatically incremented,
    // when reading their values => no need to (re)transmit them, time for transmission can be saved
    bool vIsStaticRegAddr = (mRegAddr == eRegAngle) || (mRegAddr == eRegRawAngle) || (mRegAddr == eRegMagnitude);
    if (!vIsStaticRegAddr || (aRegAddr != mRegAddr)) {
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

  template <typename T>
  inline T ReadT(uint8_t aRegAddr) const {
    T vData = 0;
    Read(aRegAddr, vData);
    return vData;
  }

  inline bool Write(uint8_t aRegAddr, const uint8_t& aData) {
    BeginTransmission(aRegAddr);
    Wire.write(aData);
    return EndTransmission();
  }

  inline bool Write(uint8_t aRegAddr, const uint16_t& aData) {
    BeginTransmission(aRegAddr);
    Wire.write(static_cast<uint8_t>((aData >> 8) & 0xFF));
    Wire.write(static_cast<uint8_t>((aData) & 0xFF));
    return EndTransmission();
  }

  const uint8_t mSlaveAddr;
  const int mDirPin;
  mutable uint8_t mRegAddr;

  static bool mAngleRequestPending;
  static uint16_t mAngle;

};

bool ggAS5600::mAngleRequestPending = false;

uint16_t ggAS5600::mAngle = 0;
