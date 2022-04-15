#include <Wire.h>
#include "ggFRAMI2C.h"


/*
template <typename TData>
void TestValueInc(ggFRAMI2C& aFRAM, uint16_t aAddr, const TData& aData)
{
  aFRAM.
}
*/

template <typename TValue>
void PrintHexT(const TValue& aValue)
{
  switch (sizeof(TValue)) {
    case 1:
      if (aValue <= 0x0F) Serial.print("0x0");
      else Serial.print("0x");
      break;
    case 2:
      if (aValue <= 0x000F) Serial.print("0x000");
      else if (aValue <= 0x00FF) Serial.print("0x00");
      else if (aValue <= 0x0FFF) Serial.print("0x0");
      else Serial.print("0x");
      break;
    default:
      Serial.print("0x");
      break;
  }
  Serial.print(aValue, HEX);
}


void Test(ggFRAMI2C& aFRAM, const char* aName)
{
  // say "hello"
  Serial.print("Executing test '");
  Serial.print(aName);
  Serial.println("'");

  // tell the I2C-address
  Serial.print("SlaveAddr: 0x");
  Serial.println(aFRAM.GetSlaveAddr(), HEX);

  // init the device (checks communication)
  bool vSuccess = aFRAM.Begin();
  if (vSuccess) Serial.print("FRAM begin success!\n");
  else Serial.print("FRAM begin failed!\n");

  // 
  Serial.println("Memory check:");
  uint16_t vAddr = 0;
  while (true) {
    if ((~vAddr & 0x03FF) == 0x03FF) PrintHexT(vAddr);
    if ((vAddr & 0x000F) == 0x000F) Serial.print(".");
    if ((vAddr & 0x03FF) == 0x03FF) Serial.println();
    //uint8_t vValue = aFRAM.Read(vAddr);
    //uint8_t vValueTest = ~vValue;
    //if (!aFRAM.Write(vAddr, vValueTest)) break;
    //if (aFRAM.Read(vAddr) != vValueTest) break;
    //aFRAM.Write(vAddr, vValue);
    uint8_t vValue = 0;
    if (!aFRAM.ReadT(vAddr, vValue)) break;
    if (vAddr < 0xFFFF) vAddr += sizeof(vValue);
    else break;
  }
  Serial.print("Max address: 0x");
  Serial.println(vAddr, HEX);  
}


ggFRAMI2C mFRAM(0x50);

#define SDA_PIN 4
#define SCL_PIN 5

void setup()
{
  // setup serial connection for debugging output
  Serial.begin(115200);
  while (!Serial) {}
  Serial.print("\nFRAM I2C Test!\n");

  // setup I2C communication IO
  // Wire.begin(); // A4: SDA, A5: SCL // arduino
  Wire.begin(SDA_PIN, SCL_PIN); // esp8266
  // Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_INT, 800000); // teensy3

  // double and float are the same types on arduino but not for ESP8266
  Serial.print("sizeof(float) = "); Serial.println(sizeof(float));  
  Serial.print("sizeof(double) = "); Serial.println(sizeof(double));  

  //
  Test(mFRAM, "FRAM A");

  /*
  uint16_t vAddr1 = 0x7fff;
  mFRAM.Write(vAddr1, mFRAM.Read(vAddr1) + 1);
  Serial.println(mFRAM.Read(vAddr1));

  uint16_t vAddr2 = 123;
  mFRAM.WriteT(vAddr2, (float)2000.8);
  mFRAM.WriteT(vAddr2, mFRAM.ReadT<float>(vAddr2) + 0.000001);
  Serial.println(mFRAM.ReadT<float>(vAddr2), 6);
  Serial.println(mFRAM.ReadT<double>(vAddr2), 6);

  uint16_t vAddr3 = 42;
  mFRAM.WriteT(vAddr3, (double)3000.1415);
  mFRAM.WriteT(vAddr3, mFRAM.ReadT<double>(vAddr3) + 0.000001);
  Serial.println(mFRAM.ReadT<double>(vAddr3), 6);
  Serial.println(mFRAM.ReadT<float>(vAddr3), 6);
  */
}


void loop()
{
}
