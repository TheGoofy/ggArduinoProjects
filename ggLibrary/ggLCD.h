#ifndef _GG_LCD_H__
#define _GG_LCD_H__

#include <Arduino.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <String.h>

#define GG_LCD_DEFAULT_ADDRESS_I2C 0x27
#define GG_LCD_DEFAULT_NUMBER_OF_ROWS 4
#define GG_LCD_DEFAULT_NUMBER_OF_COLUMNS 20

class ggLCD : public LiquidCrystal_I2C
{

public:

  ggLCD(int aAddress = GG_LCD_DEFAULT_ADDRESS_I2C);
    
  void begin(int aNumberOfColumns = GG_LCD_DEFAULT_NUMBER_OF_COLUMNS,
             int aNumberOfRows = GG_LCD_DEFAULT_NUMBER_OF_ROWS);

  void PrintValue(const String& aName,
                  int aValue,
                  int aRow,
                  int aColumnBegin = 0,
                  int aColumnEnd = -1);
  
  void PrintValue(const String& aName,
                  float aValue,
                  int aRow,
                  int aColumnBegin = 0,
                  int aColumnEnd = -1);

  void PrintValue(const String& aName,
                  int aValue,
                  const String& aUnit,
                  int aRow,
                  int aColumnBegin = 0,
                  int aColumnEnd = -1);

  void PrintValue(const String& aName,
                  float aValue,
                  const String& aUnit,
                  int aRow,
                  int aColumnBegin = 0,
                  int aColumnEnd = -1);

  void PrintProgressBar(int aProgress,
                        int aProgressBegin,
                        int aProgressEnd,
                        int aRow,
                        int aColumnBegin = 0,
                        int aColumnEnd = -1);
  
  void clearLineEnd(int aColumnEnd = -1);

  int getNumberOfColumns() const;
  int getNumberOfRows() const;
  
  virtual void home();
  virtual void setCursor(uint8_t aColumn, uint8_t aRow);
  uint8_t getCursorColumn() const;
  uint8_t getCursorRow() const;
  
protected:

  virtual size_t write(uint8_t aData);

private:

  void beginProgressBar();
  int getColumnEnd(int aColumnEnd) const;
  
  int mNumberOfRows;
  int mNumberOfColumns;
  uint8_t mCursorColumn;
  uint8_t mCursorRow;
  
};

#endif // _GG_LCD_H__
