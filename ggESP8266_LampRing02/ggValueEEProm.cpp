#define M_DEBUGGING false


#include "ggValueEEProm.h"
#include <algorithm>
#include "ggDebug.h"


void ggValueEEProm::Begin(size_t aSize)
{
  GG_DEBUG();
  GG_DEBUG_PRINTF("aSize = %d\n", aSize);
  EEPROM.begin(aSize);
  // use checksum of default-values as seed
  ChecksumSeed() = CalculateChecksumValues();
  if (EEPromDataValid()) {
    GG_DEBUG_PRINTF("EEPROM Read\n");
    ReadData();
  }
  else {
    GG_DEBUG_PRINTF("EEPROM Write\n");
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
  #if M_DEBUGGING
    Serial.printf("%s - Index=%d aSize=%d mAddressEEProm=%d EEPromNeeded=%d\n",
      __PRETTY_FUNCTION__, Values().size() - 1, aSize, mAddressEEProm, sizeof(cHeader) + ValuesSize());
    Serial.flush();
  #endif
}


bool ggValueEEProm::EEPromDataValid()
{
  cHeader vHeader;
  EEPROM.get(0, vHeader);
  // Serial.printf("%s - vHeader.mSize=%d sizeof(cHeader)+ValuesSize()=%d\n", __PRETTY_FUNCTION__, vHeader.mSize, sizeof(cHeader) + ValuesSize()); Serial.flush();
  if (vHeader.mSize == sizeof(cHeader) + ValuesSize()) {
    uint16_t vChecksumEEProm = CalculateChecksumEEProm();
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
  vHeader.mChecksum = CalculateChecksumValues();
  EEPROM.put(0, vHeader);
}


void ggValueEEProm::WriteData()
{
  std::for_each(Values().begin(), Values().end(), [] (ggValueEEProm* aValueEEProm) {
    aValueEEProm->Write(false);
  });
}


uint16_t ggValueEEProm::CalculateChecksumEEProm()
{
  uint16_t vChecksum = ChecksumSeed();
  for (int vIndex = 0; vIndex < ValuesSize(); vIndex++) {
    const int vAddressEEProm = sizeof(cHeader) + vIndex;
    uint8_t vData = EEPROM.read(vAddressEEProm);
    AddChecksum(vChecksum, vData);
  }
  return vChecksum;
}


uint16_t ggValueEEProm::CalculateChecksumValues()
{
  uint16_t vChecksum = ChecksumSeed();
  std::for_each(Values().begin(), Values().end(), [&] (ggValueEEProm* aValueEEProm) {
    uint8_t* vValuePtr = reinterpret_cast<uint8_t*>(aValueEEProm->GetValuePtr());
    for (int vIndex = 0; vIndex < aValueEEProm->GetSize(); vIndex++) {
      AddChecksum(vChecksum, *vValuePtr++);
    }
  });
  return vChecksum;
}
