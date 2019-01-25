#include "ggSampler.h"
//#include "ggButtonSimple.h"
#include "ggQuadratureDecoder.h"
#include "ggKeyPadI2C.h"
#include "ggLCD.h"
#include "ggVector.h"
#include "ggMenu.h"
#include "ggDemoMenu.h"
#include <EEPROM.h>

ggKeyPadI2C mKeyPad(0x67);
ggQuadratureDecoder mCounter(0x67);

ggLCD mLCD(0x26, 20, 4);
//ggLCD mLCD(0x27, 16, 2);

const int mAddressEEPROM = 0;


void PrintMessage(const char* aMessage, int aRow = 0)
{
  mLCD.setCursor(0, aRow);
  mLCD.print(aMessage);
  mLCD.clearLineEnd();
}


void PrintMenu(ggLCD& aLCD)
{
  static int vRowOffset = 0;
  
  int vNumberOfRows = aLCD.getNumberOfRows();
  int vNumberOfSiblings = mDemoMenu.GetNumberOfSiblings();
  int vSelectedIndex = mDemoMenu.GetItemSelectedIndex();
  
  if (vSelectedIndex < vRowOffset) {
    vRowOffset = vSelectedIndex;
  }
  
  if (vSelectedIndex >= vRowOffset + vNumberOfRows - 1) {
    vRowOffset = vSelectedIndex - vNumberOfRows + 1;
  }
  
  aLCD.noBlink();
  
  for (int vRowIndex = 0; vRowIndex < vNumberOfRows; vRowIndex++) {
    
    // draw line-by-line on the
    aLCD.setCursor(0, vRowIndex);
    
    int vSiblingIndex = vRowIndex + vRowOffset;
    if (vSiblingIndex < vNumberOfSiblings) {
      if (vSiblingIndex == vSelectedIndex) {
        aLCD.print((char)B01111110);
      }
      else {
        aLCD.print(" ");
      }
      mDemoMenu.PrintSibling(aLCD, vSiblingIndex);
    }
    
    aLCD.clearLineEnd();
    
    if (mDemoMenu.IsEditing()) {
      aLCD.setCursor(0, vSelectedIndex - vRowOffset);
      aLCD.blink();
    }
  }
}

void setup()
{
  // Serial.begin(115200);
  // Serial.println("hello goofy");
  
  mKeyPad.begin();
  mCounter.begin();
  mCounter.Set(0);

  mLCD.begin();
  mLCD.clear();
  
  PrintMessage("Start...");

  if (mKeyPad.IsOn(ggKeyPadI2C::eButtonA) && mKeyPad.IsOn(ggKeyPadI2C::eButtonC)) {
    PrintMessage("Reset Parameters...");
    mDemoMenu.put(EEPROM, mAddressEEPROM);
    delay(1000);
  }
  
  PrintMessage("Load Parameters...");
  if (!mDemoMenu.get(EEPROM, mAddressEEPROM)) {
    PrintMessage("Init Parameters...");
    mDemoMenu.put(EEPROM, mAddressEEPROM);
  }

  PrintMenu(mLCD);
}


void loop()
{
  if (mKeyPad.SwitchingOn(ggKeyPadI2C::eButtonB) ||
      mKeyPad.SwitchingOn(ggKeyPadI2C::eButtonC)) {
    // Serial.println("Menu: Enter");
    mDemoMenu.SelectEnter();
    if (mDemoMenu.IsModified()) {
      mDemoMenu.put(EEPROM, mAddressEEPROM);
      mDemoMenu.SetModified(false);
    }
    PrintMenu(mLCD);
  }
  
  if (mKeyPad.SwitchingOn(ggKeyPadI2C::eButtonA)) {
    // Serial.println("Menu: Exit");
    mDemoMenu.SelectExit();
    PrintMenu(mLCD);
  }

  static long vCounterLast = 0;
  long vCounter = mCounter.Get() / 4;
  long vCounterDelta = vCounter - vCounterLast;
  if (vCounterDelta != 0) {
    // Serial.print("Menu: Next/Prev => Value:");
    // Serial.print(vCounter);
    // Serial.print(" Delta:");
    // Serial.println(vCounterDelta);
    while (vCounterDelta > 0) {
      mDemoMenu.SelectNext();
      --vCounterDelta;
    }
    while (vCounterDelta < 0) {
      mDemoMenu.SelectPrev();
      ++vCounterDelta;
    }
    PrintMenu(mLCD);
    vCounterLast = vCounter;
  }
}
