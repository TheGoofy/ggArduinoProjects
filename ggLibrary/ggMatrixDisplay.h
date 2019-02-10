#ifndef __GG_MATRIX_DISPLAY__
#define __GG_MATRIX_DISPLAY__

#include <Arduino.h>

class ggMatrixDisplay
{
  
public:
  
  ggMatrixDisplay(byte aCols, const byte* aColPins,
                  byte aRows, const byte* aRowPins,
                  int aFramesPerSecond = 100);

  void begin();
  void run();

  void SetPixmap(const byte* aPixmap);
  
private:
  
  const byte mCols;
  const byte mRows;
  const byte* mColPins;
  const byte* mRowPins;
  
  long mMicroSecondsScanLine;
  long mMicroSecondsLastRendering;
  byte mCurrentRowIndex;
  
  const byte* mPixmap;
  
};

#endif // __GG_MATRIX_DISPLAY__
