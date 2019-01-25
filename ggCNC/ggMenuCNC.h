#ifndef __GG_MENU_CNC__
#define __GG_MENU_CNC__

#include "Arduino.h"
#include "ggMenu.h"


// *************************************************************************

void ActionMenuReturn();
const char mNameActionReturn[] PROGMEM = "RETURN";
const ggMenu::cItem mMenuItemActionReturn PROGMEM = {ggMenu::eItemAction, mNameActionReturn,
 {mAction: ActionMenuReturn}};

// *************************************************************************

const char mNameRotations[] PROGMEM = "Rotations";
const ggMenu::cItem mItemRotations PROGMEM = {ggMenu::eItemInt, mNameRotations, {mInt: {&mRotations, 1, 1, 10}}};

const char mNameNumberOfHoles[] PROGMEM = "Holes";
const ggMenu::cItem mItemNumberOfHoles PROGMEM = {ggMenu::eItemInt, mNameNumberOfHoles, {mInt: {&mNumberOfHoles, 1, 1, 1000}}};

const char mNameFirstHole[] PROGMEM = "First Hole";
const ggMenu::cItem mItemFirstHole PROGMEM = {ggMenu::eItemInt, mNameFirstHole, {mInt: {&mFirstHole, 1, 1, 1000}}};

const char mNameLastHole[] PROGMEM = "Last Hole";
const ggMenu::cItem mItemLastHole PROGMEM = {ggMenu::eItemInt, mNameLastHole, {mInt: {&mLastHole, 1, 1, 1000}}};

const char mMenuHolesName[] PROGMEM = "Holes";
const ggMenu::tItemPtr mMenuHolesItems[] PROGMEM = {
  &mItemNumberOfHoles,
  &mItemFirstHole,
  &mItemLastHole,
//  &mItemRotations,
  &mMenuItemActionReturn
};

const ggMenu::cItem mMenuHoles PROGMEM = {ggMenu::eItemMenu, mMenuHolesName,
  {mMenu: {mMenuHolesItems, sizeof(mMenuHolesItems) / sizeof(ggMenu::tItemPtr)}}};

// *************************************************************************

const char mUnitDegrees[] PROGMEM = {223, 0};
const char mUnitDegreesPerSecond[] PROGMEM = {223, '/', 's', 0};
const char mUnitDegreesPerSecondSquare[] PROGMEM = {223, '/', 's', '^', '2', 0};

const char mNameAnglePositionHome[] PROGMEM = "Home";
const ggMenu::cItem mItemAnglePositionHome PROGMEM = {ggMenu::eItemFloat, mNameAnglePositionHome, {mFloat: {&mAnglePositionHome, 0.5f, -180.0f, 180.0f, 1, mUnitDegrees}}};

const char mNameAngleSpeedInit[] PROGMEM = "Speed Init";
const ggMenu::cItem mItemAngleSpeedInit PROGMEM = {ggMenu::eItemFloat, mNameAngleSpeedInit, {mFloat: {&mAngleSpeedInit, 1.0f, 10.0f, 180.0f, 0, mUnitDegreesPerSecond}}};

const char mNameAngleSpeed[] PROGMEM = "Speed";
const ggMenu::cItem mItemAngleSpeed PROGMEM = {ggMenu::eItemFloat, mNameAngleSpeed, {mFloat: {&mAngleSpeed, 1.0f, 10.0f, 360.0f, 0, mUnitDegreesPerSecond}}};

const char mNameAngleAcceleration[] PROGMEM = "Accel";
const ggMenu::cItem mItemAngleAcceleration PROGMEM = {ggMenu::eItemFloat, mNameAngleAcceleration, {mFloat: {&mAngleAcceleration, 10.0f, 180.0f, 3600.0f, 0, mUnitDegreesPerSecondSquare}}};

const char mMenuAngleName[] PROGMEM = "Angle";
const ggMenu::tItemPtr mMenuAngleItems[] PROGMEM = {
  &mItemAnglePositionHome,
  &mItemAngleSpeedInit,
  &mItemAngleSpeed,
  &mItemAngleAcceleration,
  &mMenuItemActionReturn
};

const ggMenu::cItem mMenuAngle PROGMEM = {ggMenu::eItemMenu, mMenuAngleName,
  {mMenu: {mMenuAngleItems, sizeof(mMenuAngleItems) / sizeof(ggMenu::tItemPtr)}}};

// *************************************************************************

const char mUnitMillimeter[] PROGMEM = "mm";
const char mUnitMillimeterPerSecond[] PROGMEM = "mm/s";
const char mUnitMillimeterPerSecondSquare[] PROGMEM = "mm/s^2";

const char mNameRadiusPositionHome[] PROGMEM = "Home";
const ggMenu::cItem mItemRadiusPositionHome PROGMEM = {ggMenu::eItemFloat, mNameRadiusPositionHome, {mFloat: {&mRadiusPositionHome, 1.0f, 0.0f, 300.0f, 0, mUnitMillimeter}}};

const char mNameRadiusPositionDrillStart[] PROGMEM = "Start";
const ggMenu::cItem mItemRadiusPositionDrillStart PROGMEM = {ggMenu::eItemFloat, mNameRadiusPositionDrillStart, {mFloat: {&mRadiusPositionDrillStart, 0.5f, 0.0f, 300.0f, 1, mUnitMillimeter}}};

const char mNameRadiusPositionDrillDelta[] PROGMEM = "Delta";
const ggMenu::cItem mItemRadiusPositionDrillDelta PROGMEM = {ggMenu::eItemFloat, mNameRadiusPositionDrillDelta, {mFloat: {&mRadiusPositionDrillDelta, 0.1f, 0.0f, 20.0f, 1, mUnitMillimeter}}};

const char mNameRadiusPositionRotationDelta[] PROGMEM = "Rot.Delta";
const ggMenu::cItem mItemRadiusPositionRotationDelta PROGMEM = {ggMenu::eItemFloat, mNameRadiusPositionRotationDelta, {mFloat: {&mRadiusPositionRotationDelta, 0.1f, 0.0f, 20.0f, 1, mUnitMillimeter}}};

const char mNameRadiusSpeedInit[] PROGMEM = "Init";
const ggMenu::cItem mItemRadiusSpeedInit PROGMEM = {ggMenu::eItemFloat, mNameRadiusSpeedInit, {mFloat: {&mRadiusSpeedInit, 0.5f, 1.0f, 20.0f, 1, mUnitMillimeterPerSecond}}};

const char mNameRadiusSpeedDrilling[] PROGMEM = "Drill";
const ggMenu::cItem mItemRadiusSpeedDrilling PROGMEM = {ggMenu::eItemFloat, mNameRadiusSpeedDrilling, {mFloat: {&mRadiusSpeedDrilling, 0.01f, 0.0f, 10.0f, 2, mUnitMillimeterPerSecond}}};

const char mNameRadiusSpeed[] PROGMEM = "Normal";
const ggMenu::cItem mItemRadiusSpeed PROGMEM = {ggMenu::eItemFloat, mNameRadiusSpeed, {mFloat: {&mRadiusSpeed, 0.1f, 1.0f, 20.0f, 1, mUnitMillimeterPerSecond}}};

const char mNameRadiusAcceleration[] PROGMEM = "Accel";
const ggMenu::cItem mItemRadiusAcceleration PROGMEM = {ggMenu::eItemFloat, mNameRadiusAcceleration, {mFloat: {&mRadiusAcceleration, 10.0f, 10.0f, 1000.0f, 0, mUnitMillimeterPerSecondSquare}}};

const char mMenuXAxisName[] PROGMEM = "X-Axis";
const ggMenu::tItemPtr mMenuXAxisItems[] PROGMEM = {
  &mItemRadiusPositionHome,
  &mItemRadiusPositionDrillStart,
  &mItemRadiusPositionDrillDelta,
//  &mItemRadiusPositionRotationDelta,
  &mItemRadiusSpeedInit,
  &mItemRadiusSpeedDrilling,
  &mItemRadiusSpeed,
  &mItemRadiusAcceleration,
  &mMenuItemActionReturn
};

const ggMenu::cItem mMenuXAxis PROGMEM = {ggMenu::eItemMenu, mMenuXAxisName,
  {mMenu: {mMenuXAxisItems, sizeof(mMenuXAxisItems) / sizeof(ggMenu::tItemPtr)}}};

// *************************************************************************

const char mUnitPercent[] PROGMEM = "%";

const char mNameDrillSpeedOff[] PROGMEM = "Speed Off";
const ggMenu::cItem mItemDrillSpeedOff PROGMEM = {ggMenu::eItemInt, mNameDrillSpeedOff, {mInt: {&mDrillSpeedOff, 1, 0, 100, 0, mUnitPercent}}};

const char mNameDrillSpeedLow[] PROGMEM = "Speed Low";
const ggMenu::cItem mItemDrillSpeedLow PROGMEM = {ggMenu::eItemInt, mNameDrillSpeedLow, {mInt: {&mDrillSpeedLow, 1, 0, 100, 0, mUnitPercent}}};

const char mNameDrillSpeedHigh[] PROGMEM = "Speed High";
const ggMenu::cItem mItemDrillSpeedHigh PROGMEM = {ggMenu::eItemInt, mNameDrillSpeedHigh, {mInt: {&mDrillSpeedHigh, 1, 0, 100, 0, mUnitPercent}}};

const char mMenuDrillName[] PROGMEM = "Drill";
const ggMenu::tItemPtr mMenuDrillItems[] PROGMEM = {
  &mItemDrillSpeedOff,
  &mItemDrillSpeedLow,
  &mItemDrillSpeedHigh,
  &mMenuItemActionReturn
};

const ggMenu::cItem mMenuDrill PROGMEM = {ggMenu::eItemMenu, mMenuDrillName,
  {mMenu: {mMenuDrillItems, sizeof(mMenuDrillItems) / sizeof(ggMenu::tItemPtr)}}};

// *************************************************************************

const char mMenuSettingsName[] PROGMEM = "Settings";
const ggMenu::tItemPtr mMenuSettingsItems[] PROGMEM = {
  &mMenuHoles,
  &mMenuAngle,
  &mMenuXAxis,
  &mMenuDrill,
  &mMenuItemActionReturn
};
const ggMenu::cItem mMenuSettings PROGMEM = {ggMenu::eItemMenu, mMenuSettingsName,
  {mMenu: {mMenuSettingsItems, sizeof(mMenuSettingsItems) / sizeof(ggMenu::tItemPtr)}}};

// *************************************************************************

void ActionHome();
const char mNameActionHome[] PROGMEM = "HOME";
const ggMenu::cItem mMenuItemActionHome PROGMEM = {ggMenu::eItemAction, mNameActionHome,
  {mAction: ActionHome}};

// *************************************************************************

const char mMenuMotionName[] PROGMEM = "Motion";
const ggMenu::tItemPtr mMenuMotionItems[] PROGMEM = {
  &mMenuItemActionHome,
  &mMenuItemActionReturn
};

const ggMenu::cItem mMenuMotion PROGMEM = {ggMenu::eItemMenu, mMenuMotionName,
  {mMenu: {mMenuMotionItems, sizeof(mMenuMotionItems) / sizeof(ggMenu::tItemPtr)}}};

// *************************************************************************

void ActionRunStart();
const char mNameActionRunStart[] PROGMEM = "START";
const ggMenu::cItem mMenuItemActionRunStart PROGMEM = {ggMenu::eItemAction, mNameActionRunStart,
  {mAction: ActionRunStart}};

// *************************************************************************

void ActionRunContinue();
const char mNameActionRunContinue[] PROGMEM = "CONTINUE";
const ggMenu::cItem mMenuItemActionRunContinue PROGMEM = {ggMenu::eItemAction, mNameActionRunContinue,
  {mAction: ActionRunContinue}};

// *************************************************************************

const char mMenuMainName[] PROGMEM = "MAIN";
const ggMenu::tItemPtr mMenuMainItems[] PROGMEM = {
  &mMenuSettings,
  &mMenuMotion,
  &mMenuItemActionRunStart,
  &mMenuItemActionRunContinue
};

const ggMenu::cItem mMenuMain PROGMEM = {ggMenu::eItemMenu, mMenuMainName,
  {mMenu: {mMenuMainItems, sizeof(mMenuMainItems) / sizeof(ggMenu::tItemPtr)}}};

// *************************************************************************

ggMenu mMenuCNC(&mMenuMain, 10);

void ActionMenuReturn()
{
  mMenuCNC.SelectExit();
}

// *************************************************************************

void PrintMenu(ggLCD& aLCD)
{
  static int vMenuRowOffset = 0;
  
  int vNumberOfRows = aLCD.getNumberOfRows();
  int vNumberOfSiblings = mMenuCNC.GetNumberOfSiblings();
  int vSelectedIndex = mMenuCNC.GetItemSelectedIndex();
  
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
      mMenuCNC.PrintSibling(aLCD, vSiblingIndex);
    }
    
    aLCD.clearLineEnd();
    
    if (mMenuCNC.IsEditing()) {
      aLCD.setCursor(0, vSelectedIndex - vMenuRowOffset);
      aLCD.blink();
    }
  }
}

#endif // __GG_MENU_CNC__
