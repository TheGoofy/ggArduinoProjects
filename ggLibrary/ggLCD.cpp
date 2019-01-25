#include "Arduino.h"
#include "ggLCD.h"
#include "Wire.h"
#include "LiquidCrystal_I2C.h"


ggLCD::ggLCD(int aAddress,
             int aNumberOfColumns,
             int aNumberOfRows)
: LiquidCrystal_I2C(aAddress, aNumberOfColumns, aNumberOfRows),
  mNumberOfColumns(aNumberOfColumns),
  mNumberOfRows(aNumberOfRows),
  mCursorColumn(0),
  mCursorRow(0)
{
}


void ggLCD::begin(int aNumberOfColumns,
                  int aNumberOfRows)
{
  if (aNumberOfColumns > 0) mNumberOfColumns = aNumberOfColumns;
  if (aNumberOfRows > 0) mNumberOfRows = aNumberOfRows;
  LiquidCrystal_I2C::begin(mNumberOfColumns, mNumberOfRows);
  LiquidCrystal_I2C::init();
  LiquidCrystal_I2C::clear();
  LiquidCrystal_I2C::backlight();
  beginProgressBar();
}


void ggLCD::PrintValue(const char* aName,
                       int aValue,
                       int aRow,
                       int aColumnBegin,
                       int aColumnEnd)
{
  PrintValue(aName, aValue, "", aRow, aColumnBegin, aColumnEnd);
}


void ggLCD::PrintValue(const char* aName,
                       float aValue,
                       int aRow,
                       int aColumnBegin,
                       int aColumnEnd)
{
  PrintValue(aName, aValue, "", aRow, aColumnBegin, aColumnEnd);
}


void ggLCD::PrintValue(const char* aName,
                       int aValue,
                       const char* aUnit,
                       int aRow,
                       int aColumnBegin,
                       int aColumnEnd)
{
  setCursor(aColumnBegin, aRow);
  print(aName);
  print(": ");
  print(aValue);
  print(aUnit);
  clearLineEnd();
}


void ggLCD::PrintValue(const char* aName,
                       float aValue,
                       const char* aUnit,
                       int aRow,
                       int aColumnBegin,
                       int aColumnEnd)
{
  setCursor(aColumnBegin, aRow);
  print(aName);
  print(": ");
  print(aValue, 3);
  print(aUnit);
  clearLineEnd();
}


void ggLCD::PrintProgressBar(int aProgress,
                             int aProgressBegin,
                             int aProgressEnd,
                             int aRow,
                             int aColumnBegin,
                             int aColumnEnd)
{
  const int vColumnPixels = 5;
  const int vColumnEnd = getColumnEnd(aColumnEnd);
  const int vColumnSize = vColumnEnd - aColumnBegin;
  const int vProgressSize = aProgressEnd - aProgressBegin;
  const int vPixelProgressSize = vColumnSize * vColumnPixels;
  const int vProgressDiff = aProgress - aProgressBegin;
  const int vPixelProgress = vPixelProgressSize * (long)vProgressDiff / vProgressSize;
  const int vColumnProgress = aColumnBegin + vProgressDiff * vColumnSize / vProgressSize;
  char vChar = '?';
  setCursor(aColumnBegin, aRow);
  for (int vColumn = aColumnBegin; vColumn < vColumnEnd; vColumn++) {
    if (vColumnProgress < vColumn) vChar = 0;
    else if (vColumnProgress > vColumn) vChar = vColumnPixels;
    else vChar = vPixelProgress > 0 ? vPixelProgress % vColumnPixels : 0;
    print(vChar);
  }
}


void ggLCD::clearLineEnd(int aColumnEnd)
{
  int vColumnEnd = getColumnEnd(aColumnEnd);
  while (getCursorColumn() < vColumnEnd) print(' ');
}


void ggLCD::home()
{
  mCursorColumn = 0;
  mCursorRow = 0;
  LiquidCrystal_I2C::home();
}


void ggLCD::setCursor(uint8_t aColumn, uint8_t aRow)
{
  mCursorColumn = aColumn;
  mCursorRow = aRow;
  LiquidCrystal_I2C::setCursor(aColumn, aRow);
}


uint8_t ggLCD::getCursorColumn() const
{
  return mCursorColumn;
}


uint8_t ggLCD::getCursorRow() const
{
  return mCursorRow;
}


void ggLCD::println()
{
  clearLineEnd();
  if (mCursorRow + 1 < mNumberOfRows) {
    setCursor(0, mCursorRow + 1);
  }
  else {
    home();
  }
}


size_t ggLCD::write(uint8_t aData)
{
  mCursorColumn++;
  return LiquidCrystal_I2C::write(aData);
}


int ggLCD::getNumberOfColumns() const
{
  return mNumberOfColumns;
}


int ggLCD::getNumberOfRows() const
{
  return mNumberOfRows;
}


int ggLCD::getColumnEnd(int aColumnEnd) const
{
  return aColumnEnd < 0 ? mNumberOfColumns : aColumnEnd;
}


void ggLCD::beginProgressBar()
{
  static const uint8_t vCharBitmap[][8] = {
    {B00000, B00000, B00000, B10101, B00000, B10101, B00000, B00000},
    {B00000, B00000, B10000, B10101, B10000, B10101, B10000, B00000},
    {B00000, B00000, B11000, B11101, B11000, B11101, B11000, B00000},
    {B00000, B00000, B11100, B11101, B11100, B11101, B11100, B00000},
    {B00000, B00000, B11110, B11111, B11110, B11111, B11110, B00000},
    {B00000, B00000, B11111, B11111, B11111, B11111, B11111, B00000}
  };
  static const int vCharBitmapSize = (sizeof(vCharBitmap) / sizeof(vCharBitmap[0]));
  for (int vCharIndex = 0; vCharIndex < vCharBitmapSize; vCharIndex++) {
    createChar(vCharIndex, (uint8_t*)vCharBitmap[vCharIndex]);
  }
}
