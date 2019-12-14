#include "ggValueEEProm.h"


std::vector<uint8_t> ggValueEEProm::mData;
uint16_t ggValueEEProm::mChecksumSeed = 0;


void ggValueEEProm::Begin(size_t aSize)
{
  EEPROM.begin(aSize);
  mChecksumSeed = Checksum(0);
  if (EEPromDataValid()) {
    ReadData();
  }
  else {
    WriteHeader();
    WriteData();
    EEPROM.commit();
  }
}


ggValueEEProm::ggValueEEProm(int aSize)
{
  mIndex = mData.size();    
  mData.resize(mIndex + aSize);
}


bool ggValueEEProm::EEPromDataValid()
{
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


void ggValueEEProm::ReadData()
{
  for (int vIndex = 0; vIndex < mData.size(); vIndex++) {
    const int vAddress = sizeof(cHeader) + vIndex;
    mData[vIndex] = EEPROM.read(vAddress);
  }
}


void ggValueEEProm::WriteHeader()
{
  cHeader vHeader;
  vHeader.mSize = sizeof(cHeader) + mData.size();
  vHeader.mChecksum = Checksum(mChecksumSeed);
  EEPROM.put(0, vHeader);
}


void ggValueEEProm::WriteData()
{
  for (int vIndex = 0; vIndex < mData.size(); vIndex++) {
    const int vAddress = sizeof(cHeader) + vIndex;
    EEPROM.write(vAddress, mData[vIndex]);
  }
}


uint16_t ggValueEEProm::Checksum(uint16_t aChecksumSeed)
{
  uint16_t vChecksum = aChecksumSeed;
  for (int vIndex = 0; vIndex < mData.size(); vIndex++) {
    ChecksumAdd(vChecksum, mData[vIndex]);
  }
  return vChecksum;
}

