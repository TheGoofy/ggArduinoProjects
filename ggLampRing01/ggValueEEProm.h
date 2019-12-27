#pragma once

#include <Arduino.h>
#include <EEPROM.h>
#include <list>

class ggValueEEProm {

public:

  // reads values from eeprom or writes initial values.
  // eeprom is (re-)initialized, if ...
  // - ... number of values changes
  // - ... sequence (layput) of values changes
  // - ... any default value changes
  static void Begin(size_t aSize = 512);

protected:

  ggValueEEProm(int aSize);

  virtual int GetSize() const = 0;
  virtual void* GetValuePtr() = 0;
  virtual void Read() = 0;
  virtual void Write(bool aCommit) = 0;

  static void WriteHeader();
  
  // EEProm address of single value item
  const int mAddressEEProm;
  
private:

  struct cHeader {
    uint16_t mSize;
    uint16_t mChecksum;
  };

  static bool EEPromDataValid();
  static void ReadData();
  static void WriteData();

  static inline void AddChecksum(uint16_t& aChecksum, uint16_t aData) {
    aChecksum = (aChecksum >> 1) + ((aChecksum & 1) << 15) + aData;
  }

  static uint16_t CalculateChecksumEEProm();
  static uint16_t CalculateChecksumValues();

  typedef std::list<ggValueEEProm*> tValuesEEProm;

  static inline uint16_t& ValuesSize() {
    static uint16_t vValuesSize = 0;
    return vValuesSize;
  }

  static inline tValuesEEProm& Values() {
    static tValuesEEProm vValues;
    return vValues;
  }
  
  static inline uint16_t& ChecksumSeed() {
    static uint16_t vChecksumSeed = 0;
    return vChecksumSeed;
  }
  
};
