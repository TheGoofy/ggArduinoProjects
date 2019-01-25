#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "ggLCD.h"

const int mPinA = A0;
const int mPinB = A1;

ggLCD mLCD(0x26);

void setup()
{
  Serial.begin(115200);
  mLCD.begin(20, 4);
  pinMode(mPinA, INPUT);
  pinMode(mPinB, INPUT);
}

int mAMin = 152;
int mAMax = 982;
int mBMin = 155;
int mBMax = 984;

const int mAC = 512;
const int mARL = 166;
const int mARH = 975;
const int mAFH = 966;
const int mAFL = 206;

const int mBC = 512;
const int mBRL = 166;
const int mBRH = 968;
const int mBFH = 930;
const int mBFL = 170;

const int mD0 = mBC - mBFL;
const int mD1 = mD0 + (mARH - mARL);
const int mD2 = mD1 + (mBRH - mBRL);
const int mD3 = mD2 + (mAFH - mAFL);
const int mD4 = mD3 + (mBFH - mBFL);

const int mP = mD4 - mD0; 
const float mMillimeterPerStep = - 5.05f / (float)mP;

void PrintRange(ggLCD& aLCD,
                const String& aName,
                int aMin, int aMax,
                int aRow, int aColumnBegin, int aColumnEnd)
{
  aLCD.setCursor(aColumnBegin, aRow);
  aLCD.print(aName);
  aLCD.print(": ");
  aLCD.print(aMin);
  aLCD.print("..");
  aLCD.print(aMax);
  while (aLCD.getCursorColumn() < aColumnEnd) aLCD.print(" ");
}

void CalibrateMinMax(int aA, int aB)
{
  if (mAMin > aA) mAMin = aA;
  if (mBMin > aB) mBMin = aB;
  if (mAMax < aA) mAMax = aA;
  if (mBMax < aB) mBMax = aB;
}

int Delta(int aValue, int aMin, int aMax)
{
  int vDeltaMin = aValue - aMin;
  int vDeltaMax = aMax - aValue;
  return vDeltaMin < vDeltaMax ? vDeltaMin : vDeltaMax;
}

int mQ = 0;
int mDQ = 0;
int mStep = 0;

void loop()
{
  int vA = analogRead(mPinA);
  int vB = analogRead(mPinB);

  int vD = 0;
  if (vB < mBC) {
    if (vA < mAC) {
      // Qarter 0
      if (vA > mARL) vD = mD0 + (vA - mARL);
      else vD = mD3 + (mBFH - vB) - mP;
      if (mQ == 3) { mDQ++; mStep++; }
      if (mQ == 1) mDQ--;
      mQ = 0;
    }
    else {
      // Quarter 1
      if (vB > mBRL) vD = mD1 + (vB - mBRL);
      else vD = mD0 + (vA - mARL);
      if (mQ == 0) mDQ++;
      if (mQ == 2) mDQ--;
      mQ = 1;
    }
  }
  else {
    if (vA > mAC) {
      // Quarter 2
      if (vA < mAFH) vD = mD2 + (mAFH - vA);
      else vD = mD1 + (vB - mBRL);
      if (mQ == 1) mDQ++;
      if (mQ == 3) mDQ--;
      mQ = 2;
    }
    else {
      // Quarter 3
      if (vB < mBFH) vD = mD3 + (mBFH - vB);
      else vD = mD2 + (mAFH - vA);
      if (mQ == 2) mDQ++;
      if (mQ == 0) { mDQ--; mStep--; }
      mQ = 3;
    }
  }
  
  long vDistance = (long)mP * mStep + (long)(vD);
  
  static long vDistance0 = vDistance;

  vDistance -= vDistance0;
  float vDistaceMM = mMillimeterPerStep * vDistance;
 
  /*  
  mLCD.PrintValue("A", vA, 0, 0, 8);
  mLCD.PrintValue("B", vB, 1, 0, 8);
  mLCD.PrintProgressBar(vA, 0, 1024, 0, 8, 20);
  mLCD.PrintProgressBar(vB, 0, 1024, 1, 8, 20);
  // CalibrateMinMax(vA, vB);
  // PrintRange(mLCD, "A Range", mAMin, mAMax, 2, 0, 20);
  // PrintRange(mLCD, "B Range", mBMin, mBMax, 3, 0, 20);
  mLCD.PrintValue("DA", Delta(vA, mAMin, mAMax), 2, 0, 10);
  mLCD.PrintValue("DB", Delta(vB, mBMin, mBMax), 2, 10, 20);
  mLCD.PrintValue("D", vD, 3, 0, 8);
  mLCD.PrintProgressBar(vD, 0, mD4, 3, 8, 20);
  */
  
  /*
  Serial.print(vA);
  Serial.print(" ");
  Serial.print(vB);
  Serial.print(" ");
  Serial.print(vD);
  Serial.println();
  */

  long vMicros = micros();

  static long vLastMicros = 0;
  if (vMicros - vLastMicros > 50000) {
    
    mLCD.setCursor(0, 0);
    mLCD.print(vDistaceMM);
    mLCD.print("mm");
    while (mLCD.getCursorColumn() < 10) mLCD.print(" ");
    mLCD.PrintProgressBar(5.0f * vDistaceMM, -50, 50, 0, 10, 20);
    
    /*
    Serial.print(vD + mBC);
    Serial.print(" ");
    Serial.print(vDistaceMM);
    Serial.println();
    */
    
    //
    vLastMicros = vMicros;
  }
}
















