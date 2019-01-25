#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include "ggLCD.h"
#include "ggVector.h"
#include "ggMenu.h"
#include "ggDemoMenu.h"
#include "ggQuadratureDecoder.h"
#include "ggButtonSimple.h"
#include "ggSampler.h"


ggLCD mLCD(0x26);
ggButtonSimple mButtonEnter(2, true, true);
ggButtonSimple mButtonExit(3, true, true);
ggQuadratureDecoder mCounter(0x61);


void PrintStatus(ggLCD& aLCD, const char* aText)
{
  aLCD.setCursor(0, aLCD.getCursorRow());
  aLCD.print(aText);
  aLCD.clearLineEnd();
}


void PrintMenu()
{
  static int vMenuRowOffset = 0;
  
  int vNumberOfRows = mLCD.getNumberOfRows();
  int vNumberOfSiblings = mDemoMenu.GetNumberOfSiblings();
  int vSelectedIndex = mDemoMenu.GetItemSelectedIndex();
  
  if (vSelectedIndex < vMenuRowOffset) {
    vMenuRowOffset = vSelectedIndex;
  }
  
  if (vSelectedIndex >= vMenuRowOffset + vNumberOfRows - 1) {
    vMenuRowOffset = vSelectedIndex - vNumberOfRows + 1;
  }
  
  for (int vRowIndex = 0; vRowIndex < vNumberOfRows; vRowIndex++) {
    
    // draw line-by-line on the
    mLCD.setCursor(0, vRowIndex);
    
    int vSiblingIndex = vRowIndex + vMenuRowOffset;
    if (vSiblingIndex < vNumberOfSiblings) {
      if (vSiblingIndex == vSelectedIndex) {
        mLCD.print((char)B01111110);
      }
      else {
        mLCD.print(" ");
      }
      mDemoMenu.PrintSibling(mLCD, vSiblingIndex);
    }
    
    mLCD.clearLineEnd();
    
    if (mDemoMenu.IsEditing()) {
      mLCD.setCursor(0, vSelectedIndex - vMenuRowOffset);
      mLCD.blink();
    }
    else {
      mLCD.noBlink();
    }
  }
}


void setup()
{
  mLCD.begin();
  mLCD.clear();
  PrintStatus(mLCD, "Startup...");

  mButtonEnter.begin();
  mButtonExit.begin();
  mCounter.begin();

  ggSampler vSampler(2.0f);
  while (!vSampler.IsDue()) {
    if (mButtonEnter.IsOn() && mButtonExit.IsOn()) {
      PrintStatus(mLCD, "Reset Parameters...");
      mDemoMenu.put(EEPROM, 0);
      delay(1000);
      break;
    }
  }
  
  PrintStatus(mLCD, "Load Parameters...");
  if (!mDemoMenu.get(EEPROM, 0)) {
    PrintStatus(mLCD, "Init Parameters...");
    mDemoMenu.put(EEPROM, 0);
  }
    
  PrintMenu();
}


void loop()
{
  if (mButtonEnter.SwitchingOn()) {
    mDemoMenu.SelectEnter();
    if (mDemoMenu.IsModified()) {
      mLCD.clear(); mLCD.home();
      mDemoMenu.put(EEPROM, 0);
      mDemoMenu.SetModified(false);
    }
    PrintMenu();
  }
  
  if (mButtonExit.SwitchingOn()) {
    mDemoMenu.SelectExit();
    PrintMenu();
  }
  
  static long vCounter = mCounter.Get();
  long vCounterNew = mCounter.Get();
  long vDelta = vCounterNew - vCounter;
  
  if (vDelta > 3) {
    vCounter = vCounterNew;
    mDemoMenu.SelectNext();
    PrintMenu();
  }
  
  if (vDelta < -3) {
    vCounter = vCounterNew;
    mDemoMenu.SelectPrev();
    PrintMenu();
  }
}


