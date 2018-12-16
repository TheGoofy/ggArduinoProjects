#include "ggSampler.h"
#include "ggButtonSimple.h"
#include "ggLCD.h"
#include "ggVector.h"
#include "ggMenu.h"
#include "ggDemoMenu.h"
#include <EEPROM.h>


ggButtonSimple mButtonLeft(A3, true, true);
ggButtonSimple mButtonRight(A1, true, true);
ggButtonSimple mButtonUp(A0, true, true);
ggButtonSimple mButtonDown(A2, true, true);

ggLCD mLCD(0x26);

const int mAddressEEPROM = 0;


void PrintMessage(const char* aMessage, int aRow = 0)
{
  mLCD.setCursor(0, aRow);
  mLCD.print(aMessage);
  mLCD.clearLineEnd();
}


void PrintMenu(ggLCD& aLCD)
{
  static int vMenuRowOffset = 0;
  
  int vNumberOfRows = aLCD.getNumberOfRows();
  int vNumberOfSiblings = mDemoMenu.GetNumberOfSiblings();
  int vSelectedIndex = mDemoMenu.GetItemSelectedIndex();
  
  if (vSelectedIndex < vMenuRowOffset) {
    vMenuRowOffset = vSelectedIndex;
  }
  
  if (vSelectedIndex >= vMenuRowOffset + vNumberOfRows - 1) {
    vMenuRowOffset = vSelectedIndex - vNumberOfRows + 1;
  }
  
  aLCD.noBlink();
  
  for (int vRowIndex = 0; vRowIndex < vNumberOfRows; vRowIndex++) {
    
    // draw line-by-line on the
    aLCD.setCursor(0, vRowIndex);
    
    int vSiblingIndex = vRowIndex + vMenuRowOffset;
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
      aLCD.setCursor(0, vSelectedIndex - vMenuRowOffset);
      aLCD.blink();
    }
  }
}

void setup()
{
  mButtonLeft.begin();
  mButtonRight.begin();
  mButtonUp.begin();
  mButtonDown.begin();

  mLCD.begin(20, 4);
  mLCD.clear();
  
  PrintMessage("Start...");

  if (mButtonLeft.IsOn() && mButtonRight.IsOn()) {
    PrintMessage("Reset Parameters...");
    mDemoMenu.put(EEPROM, mAddressEEPROM);
    delay(1000);
  }
  
  PrintMessage("Load Parameters...");
  if (!mDemoMenu.get(EEPROM, mAddressEEPROM)) {
    PrintMessage("Init Parameters...");
    mDemoMenu.put(EEPROM, mAddressEEPROM);
  }

  mDemoMenu.SelectEnter();
  PrintMenu(mLCD);
}


void loop()
{
  if (mButtonRight.SwitchingOn()) {
    mDemoMenu.SelectEnter();
    if (mDemoMenu.IsModified()) {
      mLCD.clear(); mLCD.home();
      mDemoMenu.put(EEPROM, mAddressEEPROM);
      mDemoMenu.SetModified(false);
    }
    PrintMenu(mLCD);
  }
  
  if (mButtonLeft.SwitchingOn()) {
    if (mDemoMenu.GetItemSelectedLevel() > 1) {
      mDemoMenu.SelectExit();
      PrintMenu(mLCD);
    }
  }
  
  if (mButtonUp.SwitchingOn()) {
    mDemoMenu.SelectNext();
    PrintMenu(mLCD);
  }
  
  if (mButtonDown.SwitchingOn()) {
    mDemoMenu.SelectPrev();
    PrintMenu(mLCD);
  }
}
