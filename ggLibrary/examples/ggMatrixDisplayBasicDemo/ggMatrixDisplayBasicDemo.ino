#include "ggMatrixDisplay.h"

const byte mCols = 8;
const byte mRows = 8;
const byte mColPins[mCols] = {4,A3,6,A2,11,7,13,8};
const byte mRowPins[mRows] = {12,A4,A5,9,5,10,A0,A1};

const byte mPixmapA[8] = {
  B11111111,
  B11100111,
  B11100111,
  B10000001,
  B10000001,
  B11100111,
  B11100111,
  B11111111
};

const byte mPixmapB[8] = {
  B01000010,
  B11100111,
  B01111110,
  B00111100,
  B00111100,
  B01111110,
  B11100111,
  B01000010,
};

ggMatrixDisplay mDisplay(mCols, mColPins,
                         mRows, mRowPins);

void setup()
{
  mDisplay.begin();
}

void loop()
{
  // alternate pixmap A or B
  if ((millis() / 500) % 2) {
    mDisplay.SetPixmap(mPixmapA);
  }
  else {
    mDisplay.SetPixmap(mPixmapB);
  }
  
  // rendering (call this as often as possible)
  mDisplay.run();  
}

