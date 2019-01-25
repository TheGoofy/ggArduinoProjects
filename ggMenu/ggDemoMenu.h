#ifndef __GG_DEMO_MENU__
#define __GG_DEMO_MENU__

#include "Arduino.h"
#include "ggMenu.h"

// *************************************************************************

byte mTemperature = 80;
const char mTemperatureName[] PROGMEM = "Temperature";
const char mTemperatureUnit[] PROGMEM = {223, 'C', 0};
const ggMenu::cItem mMenuItemTemperature PROGMEM = {
  mType: ggMenu::eItemByte,
  mName: mTemperatureName,
  mValue: {
    mByte: {
      mValue: &mTemperature,
      mStep: 5,
      mMin: 60,
      mMax: 240,
      mDecimals: 0,
      mUnit: mTemperatureUnit
    }
  }
};

// *************************************************************************

float mVoltage = 3.7f;
const char mVoltageName[] PROGMEM = "Voltage";
const char mVoltageUnit[] PROGMEM = "V";
const ggMenu::cItem mMenuItemVoltage PROGMEM = {
  mType: ggMenu::eItemFloat,
  mName: mVoltageName,
  mValue: {
    mFloat: {
      mValue: &mVoltage,
      mStep: 0.1f,
      mMin: 0.0f,
      mMax: 50.0f,
      mDecimals: 1,
      mUnit: mVoltageUnit
    }
  }
};

// *************************************************************************

enum tBatteryType {
  eBatteryUnknown,
  eBatteryLiIo,
  eBatteryLiPo,
  eBatteryNiMh,
  eBatteryPb
};
tBatteryType mBatteryType = eBatteryLiIo;
const char mBatteryName[] PROGMEM = "Battery";
const char mBatteryTypeNameUnknown[] PROGMEM = "Unknown";
const char mBatteryTypeNameLiIo[] PROGMEM = "LiIo";
const char mBatteryTypeNameLiPo[] PROGMEM = "LiPo";
const char mBatteryTypeNameNiMh[] PROGMEM = "NiMh";
const char mBatteryTypeNamePb[] PROGMEM = "Pb";
const ggMenu::tStringPtr mBatteryTypeNames[] PROGMEM = {
  mBatteryTypeNameUnknown,
  mBatteryTypeNameLiIo,
  mBatteryTypeNameLiPo,
  mBatteryTypeNameNiMh,
  mBatteryTypeNamePb
};
const ggMenu::cItem mMenuItemBattery PROGMEM = {
  mType: ggMenu::eItemEnum,
  mName: mBatteryName,
  mValue: {
    mEnum: {
      mIndex: (byte*)&mBatteryType,
      mNames: mBatteryTypeNames,
      mSize: sizeof(mBatteryTypeNames) / sizeof(ggMenu::tStringPtr)
    }
  }
};

// *************************************************************************

void ActionReturn();
const char mNameActionReturn[] PROGMEM = "RETURN";
const ggMenu::cItem mMenuItemActionReturn PROGMEM = {
  mType: ggMenu::eItemAction,
  mName: mNameActionReturn,
  mValue: {
    mAction: ActionReturn
  }
};

// *************************************************************************

const char mMenuSettingsName[] PROGMEM = "SETTINGS";
const ggMenu::tItemPtr mMenuSettingsItems[] PROGMEM = {
  &mMenuItemTemperature,
  &mMenuItemVoltage,
  &mMenuItemBattery,
  &mMenuItemActionReturn
};
const ggMenu::cItem mMenuSettings PROGMEM = {
  mType: ggMenu::eItemMenu,
  mName: mMenuSettingsName,
  mValue: {
    mMenu: {
      mItems: mMenuSettingsItems,
      mSize: sizeof(mMenuSettingsItems) / sizeof(ggMenu::tItemPtr)
    }
  }
};

// *************************************************************************

int mA =  1; const char mNameA[] PROGMEM = "a"; const ggMenu::cItem mItemA PROGMEM = {ggMenu::eItemInt, mNameA, {mInt: {&mA, 1, -20, 20}}};
int mB =  2; const char mNameB[] PROGMEM = "b"; const ggMenu::cItem mItemB PROGMEM = {ggMenu::eItemInt, mNameB, {mInt: {&mB, 1, -20, 20}}};
int mC =  3; const char mNameC[] PROGMEM = "c"; const ggMenu::cItem mItemC PROGMEM = {ggMenu::eItemInt, mNameC, {mInt: {&mC, 1, -20, 20}}};
int mD =  4; const char mNameD[] PROGMEM = "d"; const ggMenu::cItem mItemD PROGMEM = {ggMenu::eItemInt, mNameD, {mInt: {&mD, 1, -20, 20}}};
int mE =  5; const char mNameE[] PROGMEM = "e"; const ggMenu::cItem mItemE PROGMEM = {ggMenu::eItemInt, mNameE, {mInt: {&mE, 1, -20, 20}}};
int mF =  6; const char mNameF[] PROGMEM = "f"; const ggMenu::cItem mItemF PROGMEM = {ggMenu::eItemInt, mNameF, {mInt: {&mF, 1, -20, 20}}};
int mG =  7; const char mNameG[] PROGMEM = "g"; const ggMenu::cItem mItemG PROGMEM = {ggMenu::eItemInt, mNameG, {mInt: {&mG, 1, -20, 20}}};
int mH =  8; const char mNameH[] PROGMEM = "h"; const ggMenu::cItem mItemH PROGMEM = {ggMenu::eItemInt, mNameH, {mInt: {&mH, 1, -20, 20}}};
int mI =  9; const char mNameI[] PROGMEM = "i"; const ggMenu::cItem mItemI PROGMEM = {ggMenu::eItemInt, mNameI, {mInt: {&mI, 1, -20, 20}}};
int mJ = 10; const char mNameJ[] PROGMEM = "j"; const ggMenu::cItem mItemJ PROGMEM = {ggMenu::eItemInt, mNameJ, {mInt: {&mJ, 1, -20, 20}}};
int mK = 11; const char mNameK[] PROGMEM = "k"; const ggMenu::cItem mItemK PROGMEM = {ggMenu::eItemInt, mNameK, {mInt: {&mK, 1, -20, 20}}};
int mL = 12; const char mNameL[] PROGMEM = "l"; const ggMenu::cItem mItemL PROGMEM = {ggMenu::eItemInt, mNameL, {mInt: {&mL, 1, -20, 20}}};

const char mNameAF[] PROGMEM = "ITEMS A..F";
const ggMenu::tItemPtr mMenuItemsAF[] PROGMEM = {&mItemA, &mItemB, &mItemC, &mItemD, &mItemE, &mItemF, &mMenuItemActionReturn};
const ggMenu::cItem mMenuAF PROGMEM = {ggMenu::eItemMenu, mNameAF, {mMenu: {mMenuItemsAF, sizeof(mMenuItemsAF)/sizeof(ggMenu::tItemPtr)}}};

const char mNameGL[] PROGMEM = "ITEMS G..L";
const ggMenu::tItemPtr mMenuItemsGL[] PROGMEM = {&mItemG, &mItemH, &mItemI, &mItemJ, &mItemK, &mItemL, &mMenuItemActionReturn};
const ggMenu::cItem mMenuGL PROGMEM = {ggMenu::eItemMenu, mNameGL, {mMenu: {mMenuItemsGL, sizeof(mMenuItemsGL)/sizeof(ggMenu::tItemPtr)}}};

// *************************************************************************

const char mMenuMainName[] PROGMEM = "MAIN";
const ggMenu::tItemPtr mMenuMainItems[] PROGMEM = {&mMenuSettings, &mMenuAF, &mMenuGL, &mItemA, &mItemB, &mItemC, &mMenuItemActionReturn};
const ggMenu::cItem mMenuMain PROGMEM = {ggMenu::eItemMenu, mMenuMainName, {mMenu: {mMenuMainItems, sizeof(mMenuMainItems)/sizeof(ggMenu::tItemPtr)}}};

// *************************************************************************

ggMenu mDemoMenu(&mMenuMain);

// *************************************************************************

void ActionReturn()
{
  mDemoMenu.SelectExit();
}

// *************************************************************************

#endif // __GG_DEMO_MENU__
