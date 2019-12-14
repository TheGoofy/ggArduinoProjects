#pragma once

#include <EEPROM.h>
#include <vector>

class ggValueEEProm {

public:

  static void Begin(size_t aSize = 512);

protected:

  ggValueEEProm(int aSize);

  template <typename TValueType>
  inline TValueType& Value() {
    return *reinterpret_cast<TValueType*>(&mData[mIndex]);
  }

  template <typename TValueType>
  inline const TValueType& Value() const {
    return *reinterpret_cast<const TValueType*>(&mData[mIndex]);
  }

  template <typename TValueType>
  void Write() const {
    WriteHeader();
    const int vAddress = sizeof(cHeader) + mIndex;
    EEPROM.put(vAddress, Value<TValueType>());
    EEPROM.commit();
  }

private:

  struct cHeader {
    uint16_t mSize;
    uint16_t mChecksum;
  };

  static bool EEPromDataValid();
  static void ReadData();
  static void WriteHeader();
  static void WriteData();

  static inline void ChecksumAdd(uint16_t& aChecksum, uint16_t aData) {
    aChecksum = (aChecksum >> 1) + ((aChecksum & 1) << 15) + aData;
  }

  static uint16_t Checksum(uint16_t aChecksumSeed);

  // allocate RAM-data as byte array
  static std::vector<uint8_t> mData;
  static uint16_t mChecksumSeed;

  // RAM "address" of single data item
  int mIndex;
  
};

