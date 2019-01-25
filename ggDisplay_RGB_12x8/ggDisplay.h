#ifndef __DISPLAY_H__
#define __DISPLAY_H__

//#define PINOUT_V0
#define PINOUT_V1

class ggDisplay
{

  #define M_SIZE_X 12
  #define M_SIZE_Y 8  

public:

  ggDisplay(unsigned long aScreenDelayMillis = 8, int aBrightnessSteps = 8)
  : mSizeX(M_SIZE_X),
    mSizeY(M_SIZE_Y),
#ifdef PINOUT_V0    
    mColPins({A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11}),
    mRowPinsR({22, 23, 24, 25, 26, 27, 28, 29}), // PORT A
    mRowPinsG({30, 31, 32, 33, 34, 35, 36, 37}), // Port C
    mRowPinsB({42, 43, 44, 45, 46, 47, 48, 49}), // Port L
#endif // PINOUT_V0
#ifdef PINOUT_V1
    mColPins({A11, A10, A9, A8, A7, A6, A5, A4, A3, A2, A1, A0}),
    mRowPinsR({37, 35, 33, 31, 29, 27, 25, 23}),
    mRowPinsG({22, 24, 26, 28, 30, 32, 34, 36}),
    mRowPinsB({6, 7, 8, 9, 10, 11, 12, 13}),
#endif // PINOUT_V1
    mBrightnessSteps(aBrightnessSteps),
    mSubsampledSizeX(M_SIZE_X * (aBrightnessSteps - 1)),
    mSubsampledColDelayMicros(1000 * aScreenDelayMillis / M_SIZE_X / (aBrightnessSteps - 1)),
    mMicrosLast(0),
    mSubsampledIndexX(0)
  {
    mRowsR = new unsigned char[mSubsampledSizeX];
    mRowsG = new unsigned char[mSubsampledSizeX];
    mRowsB = new unsigned char[mSubsampledSizeX];
    Clear();
  }

  virtual ~ggDisplay() {
    delete[] mRowsR;
    delete[] mRowsG;
    delete[] mRowsB;
  }

  void setup() {
    for (int vX = 0; vX < mSizeX; vX++) {
      pinMode(mColPins[vX], OUTPUT);
      SetCol(vX, false);
    }
    for (int vY = 0; vY < mSizeY; vY++) {
      pinMode(mRowPinsR[vY], OUTPUT);
      pinMode(mRowPinsG[vY], OUTPUT);
      pinMode(mRowPinsB[vY], OUTPUT);
      SetRowRGB(vY, false, false, false);
    }
  }

  int run() {
    long vMicros = micros();
    if (vMicros - mMicrosLast > mSubsampledColDelayMicros) {
      // turn off current column
      int vIndexX = mSubsampledIndexX / (mBrightnessSteps - 1);
      SetCol(vIndexX, false);
      // advance subsampled column
      ++mSubsampledIndexX;
      if (mSubsampledIndexX >= mSubsampledSizeX) mSubsampledIndexX = 0;
      vIndexX = mSubsampledIndexX / (mBrightnessSteps - 1);
      // set new row bits
      SetRowsRGB(mRowsR[mSubsampledIndexX],
                 mRowsG[mSubsampledIndexX],
                 mRowsB[mSubsampledIndexX]);
      // turn on (new) column
      SetCol(vIndexX, true);
      // remember time for next sub-sampling
      mMicrosLast = vMicros;
    }
  }

  inline int GetSizeX() const {
    return mSizeX;
  }

  inline int GetSizeY() const {
    return mSizeY;
  }

  void SetPixelRGB(int aX, int aY, unsigned char aR, unsigned char aG, unsigned char aB) {
    unsigned char vDivisor = 256 / mBrightnessSteps;
    unsigned char vR = aR / vDivisor;
    unsigned char vG = aG / vDivisor;
    unsigned char vB = aB / vDivisor;
    unsigned char vMaskR = B00000001 << aY;
    unsigned char vMaskG = B10000000 >> aY;
    unsigned char vMaskB = B10000000 >> aY;
    int vSubsampleBeginX = aX * (mBrightnessSteps - 1);
    int vSubsampleDeltaX = 0;
    while (vSubsampleDeltaX < mBrightnessSteps - 1) {
      int vSubsampleIndexX = vSubsampleBeginX + vSubsampleDeltaX;
      unsigned char& vRowsR = mRowsR[vSubsampleIndexX];
      vRowsR = vR > vSubsampleDeltaX ? vRowsR | vMaskR : vRowsR & ~vMaskR;
      unsigned char& vRowsG = mRowsG[vSubsampleIndexX];
      vRowsG = vG > vSubsampleDeltaX ? vRowsG | vMaskG : vRowsG & ~vMaskG;
      unsigned char& vRowsB = mRowsB[vSubsampleIndexX];
      vRowsB = vB > vSubsampleDeltaX ? vRowsB | vMaskB : vRowsB & ~vMaskB;
      ++vSubsampleDeltaX;
    }
  }

  inline void Clear() {
    memset(mRowsR, 0, mSubsampledSizeX);
    memset(mRowsG, 0, mSubsampledSizeX);
    memset(mRowsB, 0, mSubsampledSizeX);
  }

  inline void TurnOff() {
    SetRowsRGB(0, 0, 0);
    for (int vX = 0; vX < mSizeX; vX++) SetCol(vX, false);
  }


private:

  inline void SetCol(int aX, bool aOn) {
#ifdef PINOUT_V0
    digitalWrite(mColPins[aX], !aOn);
#endif // PINOUT_V0
#ifdef PINOUT_V1
    digitalWrite(mColPins[aX], aOn);
#endif // PINOUT_V1
  }

  inline void SetRowsRGB(unsigned char aR, unsigned char aG, unsigned char aB) {
#ifdef PINOUT_V0
    PORTA = aR;
    PORTC = aG;
    PORTL = aB;
#endif // PINOUT_V0
#ifdef PINOUT_V1
    unsigned char vMask = B10000000;
    for (int vY = 0; vY < mSizeY; vY++) {
      SetRowRGB(vY, vMask & aR,
                    vMask & aG,
                    vMask & aB);
      vMask >>= 1;
    }
#endif // PINOUT_V1
  }

  inline void SetRowR(int aY, bool aR) {
    digitalWrite(mRowPinsR[aY], aR);
  }

  inline void SetRowG(int aY, bool aG) {
    digitalWrite(mRowPinsG[aY], aG);
  }

  inline void SetRowB(int aY, bool aB) {
    digitalWrite(mRowPinsB[aY], aB);
  }

  inline void SetRowRGB(int aY, bool aR, bool aG, bool aB) {
    digitalWrite(mRowPinsR[aY], aR);
    digitalWrite(mRowPinsG[aY], aG);
    digitalWrite(mRowPinsB[aY], aB);
  }

private:

  const int mSizeX;
  const int mSizeY;

  const unsigned char mColPins[M_SIZE_X];

  const unsigned char mRowPinsR[M_SIZE_Y];
  const unsigned char mRowPinsG[M_SIZE_Y];
  const unsigned char mRowPinsB[M_SIZE_Y];

  const int mBrightnessSteps;
  const int mSubsampledSizeX;
  const unsigned long mSubsampledColDelayMicros;
  
  unsigned char* mRowsR;
  unsigned char* mRowsG;
  unsigned char* mRowsB;

  unsigned long mMicrosLast;
  int mSubsampledIndexX;  

};

#endif // __DISPLAY_H__
