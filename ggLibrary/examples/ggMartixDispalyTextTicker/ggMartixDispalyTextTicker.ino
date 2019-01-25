#include "ggMatrixDisplay.h"
#include "ggSampler.h"
#include "ggFontBold.h"

const byte mCols = 8;
const byte mRows = 8;
const byte mColPins[mCols] = {4,A3,6,A2,11,7,13,8};
const byte mRowPins[mRows] = {12,A4,A5,9,5,10,A0,A1};

ggMatrixDisplay mDisplay(mCols, mColPins,
                         mRows, mRowPins);

ggFont* mFont = new ggFontBold();

ggSampler mTickerSampler(15.0f);

//char mTickerText[] = "Hello World! ";
//char mTickerText[] = {'V', 'o', 'l', ':', ' ', '4', '.', '2', '7', 'c', 'm', 131, ' ', 0};
char mTickerText[] = "Fabian, Lars, Claudia und Christoph wohnen in Hedingen! ";
byte mTickerTextLength = sizeof(mTickerText) - 1;

char mChar = ' ';
byte mCharPixelShift = 0;
byte mPixmap[8];

void setup()
{
  mDisplay.begin();
  mDisplay.SetPixmap(mPixmap);
}

void loop()
{
  // render the ticker pixmap
  if (mTickerSampler.IsDue()) {
    byte vCharWidth = mFont->GetCharWidth(mChar);
    const byte* vCharPixels = mFont->GetCharPixels(mChar);
    for (byte vRowIndex = 0; vRowIndex < 8; vRowIndex++) {
      mPixmap[vRowIndex] <<= 1;
      if (mCharPixelShift < vCharWidth) {
        boolean vBit = (vCharPixels[7-vRowIndex] >> (vCharWidth - 1 - mCharPixelShift)) & 1;
        mPixmap[vRowIndex] |= vBit;
      }
    }
    if (mCharPixelShift < vCharWidth) {
      mCharPixelShift++;
    }
    else {
      mCharPixelShift = 0;
      static byte vIndex = 0;
      mChar = mTickerText[vIndex];
      vIndex = (vIndex + 1) % mTickerTextLength;
    }
  }
  
  // display the pixmap
  mDisplay.run();
}
