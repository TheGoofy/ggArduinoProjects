#include "ggMatrixDisplay.h"


ggMatrixDisplay::ggMatrixDisplay(byte aCols, const byte* aColPins,
                                 byte aRows, const byte* aRowPins,
                                 int aFramesPerSecond)
: mCols(aCols),
  mRows(aRows),
  mColPins(aColPins),
  mRowPins(aRowPins),
  mMicroSecondsScanLine(1000000/(aRows*aFramesPerSecond)),
  mMicroSecondsLastRendering(0),
  mCurrentRowIndex(0),
  mPixmap(0)
{
}


void ggMatrixDisplay::begin()
{
  for (byte vColIndex = 0; vColIndex < mCols; vColIndex++) {
    pinMode(mColPins[vColIndex], OUTPUT);
  }
  for (byte vRowIndex = 0; vRowIndex < mRows; vRowIndex++) {
    pinMode(mRowPins[vRowIndex], INPUT);
  }
}


void ggMatrixDisplay::SetPixmap(const byte* aPixmap)
{
  mPixmap = aPixmap;
}


void ggMatrixDisplay::run()
{
  if (mPixmap != 0) {
    long vMicroSeconds = micros();
    if (vMicroSeconds - mMicroSecondsLastRendering >= mMicroSecondsScanLine) {
      mMicroSecondsLastRendering = vMicroSeconds;
      byte vNextRowIndex = (mCurrentRowIndex + 1) % mRows;
      byte vLinePixels = mPixmap[vNextRowIndex];
      // switch off current (old) row
      pinMode(mRowPins[mCurrentRowIndex], INPUT);
      // set output for each column pixel
      for (byte vColIndex = 0; vColIndex < mCols; vColIndex++) {
        digitalWrite(mColPins[vColIndex], vLinePixels & 1);
        vLinePixels >>= 1;
      }
      // turn on the new line
      pinMode(mRowPins[vNextRowIndex], OUTPUT);
      // next row ...
      mCurrentRowIndex = vNextRowIndex;
    }
  }
}
