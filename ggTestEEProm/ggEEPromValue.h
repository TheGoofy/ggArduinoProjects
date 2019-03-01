#pragma once

#include <EEPROM.h>
#include <vector>

class ggEEPromValue {

public:

  ggEEPromValue(int aSize) {
    mIndex = mData.size();    
    mData.resize(mIndex + aSize);
  }

  static void Begin(size_t aSize = 512) {
    EEPROM.begin(aSize);
    mChecksumSeed = Checksum(0);
    if (Valid()) {
      ReadData();
    }
    else {
      WriteHeader();
      WriteData();
      EEPROM.commit();
    }
  }

protected:

  template <typename TValueType>
  inline TValueType& Value() {
    return *reinterpret_cast<TValueType*>(&mData[mIndex]);
  }

  template <typename TValueType>
  inline const TValueType& Value() const {
    return *reinterpret_cast<const TValueType*>(&mData[mIndex]);
  }

  template <typename TValueType>
  void Write(const TValueType& aValue) const {
    WriteHeader();
    const int vAddress = sizeof(cHeader) + mIndex;
    EEPROM.put(vAddress, aValue);
    EEPROM.commit();
  }

private:

  struct cHeader {
    uint16_t mSize;
    uint16_t mChecksum;
  };

  static bool Valid() {
    cHeader vHeader;
    EEPROM.get(0, vHeader);
    if (vHeader.mSize == sizeof(cHeader) + mData.size()) {
      uint16_t vChecksum = mChecksumSeed;
      for (int vIndex = 0; vIndex < mData.size(); vIndex++) {
        const int vAddress = sizeof(cHeader) + vIndex;
        uint8_t vData = EEPROM.read(vAddress);
        ChecksumAdd(vChecksum, vData);
      }
      return vHeader.mChecksum == vChecksum;
    }
    return false;
  }

  static void ReadData() {
    for (int vIndex = 0; vIndex < mData.size(); vIndex++) {
      const int vAddress = sizeof(cHeader) + vIndex;
      mData[vIndex] = EEPROM.read(vAddress);
    }
  }

  static void WriteHeader() {
    cHeader vHeader;
    vHeader.mSize = sizeof(cHeader) + mData.size();
    vHeader.mChecksum = Checksum(mChecksumSeed);
    EEPROM.put(0, vHeader);
  }

  static void WriteData() {
    for (int vIndex = 0; vIndex < mData.size(); vIndex++) {
      const int vAddress = sizeof(cHeader) + vIndex;
      EEPROM.write(vAddress, mData[vIndex]);
    }
  }

  static inline void ChecksumAdd(uint16_t& aChecksum, uint16_t aData) {
    aChecksum = (aChecksum >> 1) + ((aChecksum & 1) << 15) + aData;
  }

  static uint16_t Checksum(uint16_t aChecksumSeed) {
    uint16_t vChecksum = aChecksumSeed;
    for (int vIndex = 0; vIndex < mData.size(); vIndex++) {
      ChecksumAdd(vChecksum, mData[vIndex]);
    }
    return vChecksum;
  }

  // allocate RAM-data as byte array
  static std::vector<uint8_t> mData;
  static uint16_t mChecksumSeed;

  // RAM "address" of single data item
  int mIndex;
  
};

std::vector<uint8_t> ggEEPromValue::mData;
uint16_t ggEEPromValue::mChecksumSeed = 0;

