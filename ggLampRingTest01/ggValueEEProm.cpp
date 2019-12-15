#include "ggValueEEProm.h"
#include <algorithm>


void ggValueEEProm::Begin(size_t aSize)
{
  EEPROM.begin(aSize);
  ChecksumSeed() = ChecksumValues();
  if (EEPromDataValid()) {
    // Serial.printf("%s - EEPROM Read\n", __PRETTY_FUNCTION__); Serial.flush();
    ReadData();
  }
  else {
    // Serial.printf("%s - EEPROM Write\n", __PRETTY_FUNCTION__); Serial.flush();
    WriteHeader();
    WriteData();
    EEPROM.commit();
  }
}


ggValueEEProm::ggValueEEProm(int aSize)
: mAddressEEProm(sizeof(cHeader) + ValuesSize())
{
  ValuesSize() += aSize;
  Values().push_back(this);
  // Serial.printf("%s - mAddressEEProm=%d ValuesSize()=%d Values().size()=%d\n", __PRETTY_FUNCTION__, mAddressEEProm, ValuesSize(), Values().size()); Serial.flush();
}


bool ggValueEEProm::EEPromDataValid()
{
  cHeader vHeader;
  EEPROM.get(0, vHeader);
  // Serial.printf("%s - vHeader.mSize=%d sizeof(cHeader)+ValuesSize()=%d\n", __PRETTY_FUNCTION__, vHeader.mSize, sizeof(cHeader) + ValuesSize()); Serial.flush();
  if (vHeader.mSize == sizeof(cHeader) + ValuesSize()) {
    uint16_t vChecksumEEProm = ChecksumEEProm();
    // Serial.printf("%s - vHeader.mChecksum=%d vChecksumEEProm=%d\n", __PRETTY_FUNCTION__, vHeader.mChecksum, vChecksumEEProm); Serial.flush();
    return vHeader.mChecksum == vChecksumEEProm;
  }
  return false;
}


void ggValueEEProm::ReadData()
{
  std::for_each(Values().begin(), Values().end(), [] (ggValueEEProm* aValueEEProm) {
    aValueEEProm->Read();
  });
}


void ggValueEEProm::WriteHeader()
{
  cHeader vHeader;
  vHeader.mSize = sizeof(cHeader) + ValuesSize();
  vHeader.mChecksum = ChecksumValues();
  EEPROM.put(0, vHeader);
}


void ggValueEEProm::WriteData()
{
  std::for_each(Values().begin(), Values().end(), [] (ggValueEEProm* aValueEEProm) {
    aValueEEProm->Write(false);
  });
}


uint16_t ggValueEEProm::ChecksumEEProm()
{
  uint16_t vChecksum = ChecksumSeed();
  for (int vIndex = 0; vIndex < ValuesSize(); vIndex++) {
    const int vAddressEEProm = sizeof(cHeader) + vIndex;
    uint8_t vData = EEPROM.read(vAddressEEProm);
    ChecksumAdd(vChecksum, vData);
  }
  return vChecksum;
}


uint16_t ggValueEEProm::ChecksumValues()
{
  uint16_t vChecksum = ChecksumSeed();
  std::for_each(Values().begin(), Values().end(), [&] (ggValueEEProm* aValueEEProm) {
    uint8_t* vValuePtr = reinterpret_cast<uint8_t*>(aValueEEProm->GetValuePtr());
    for (int vIndex = 0; vIndex < aValueEEProm->GetSize(); vIndex++) {
      ChecksumAdd(vChecksum, *vValuePtr++);
    }
  });
  return vChecksum;
}
