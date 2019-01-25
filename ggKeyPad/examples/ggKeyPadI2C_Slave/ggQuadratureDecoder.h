#ifndef __GG_QUADRATURE_DECODER__
#define __GG_QUADRATURE_DECODER__

#include <Arduino.h>

unsigned char mQuadratureDecoderPinA;
unsigned char mQuadratureDecoderPinB;
boolean mQuadratureDecoderValueA;
boolean mQuadratureDecoderValueB;
long mQuadratureDecoderCounter;

class ggQuadratureDecoder
{

public:

  ggQuadratureDecoder()
  : mEnable(false),
    mSpeedSamplingTime(100000), // 100'000 microseconds => 10 samples per second
    mSpeedLastMicros(0),
    mSpeedLastCounter(0),
    mSpeed(0)
  {
  }

  void begin(unsigned char aPinA,
             unsigned char aPinB) {
    mQuadratureDecoderValueA = false;
    mQuadratureDecoderValueB = false;
    mQuadratureDecoderCounter = 0;
    mQuadratureDecoderPinA = aPinA;
    mQuadratureDecoderPinB = aPinB;
    pinMode(mQuadratureDecoderPinA, INPUT_PULLUP);
    pinMode(mQuadratureDecoderPinB, INPUT_PULLUP);
    enable();
  }

  void run() {
    if (mEnable) {
      long vMicros = micros();
      if (vMicros - mSpeedLastMicros >= mSpeedSamplingTime) {
        long vCounter = mQuadratureDecoderCounter;
        mSpeed = vCounter - mSpeedLastCounter;
        mSpeedLastMicros = vMicros;
        mSpeedLastCounter = vCounter;
      }
    }
  }

  void enable(bool aEnable) {
    aEnable ? enable() : disable();
  }

  void enable() {
    if (!mEnable) {
      mEnable = true;
      attachInterrupt(0, ChangeValueA, CHANGE);
      attachInterrupt(1, ChangeValueB, CHANGE);
    }
  }

  void disable() {
    if (mEnable) {
      mEnable = false;
      detachInterrupt(0);
      detachInterrupt(1);
    }
  }

  bool getEnable() const {
    return mEnable;
  }

  void setCounter(long aCounter) {
    mQuadratureDecoderCounter = aCounter;
  }

  long getCounter() const {
    return mQuadratureDecoderCounter;
  }

  long getSpeed() const {
    return mSpeed;
  }

  void setSpeedSamplingTime(long aMicros) {
    mSpeedSamplingTime = aMicros;
  }

  long getSpeedSamplingTime() const {
    return mSpeedSamplingTime;
  }

private:

  static void ChangeValueA() {
    if (mQuadratureDecoderValueA != digitalRead(mQuadratureDecoderPinA)) {
      mQuadratureDecoderValueA = !mQuadratureDecoderValueA;
      mQuadratureDecoderCounter += mQuadratureDecoderValueA ^ mQuadratureDecoderValueB ? -1 : 1;
    }
  }

  static void ChangeValueB() {
    if (mQuadratureDecoderValueB != digitalRead(mQuadratureDecoderPinB)) {
      mQuadratureDecoderValueB = !mQuadratureDecoderValueB;
      mQuadratureDecoderCounter += mQuadratureDecoderValueA ^ mQuadratureDecoderValueB ? 1 : -1;
    }
  }

  bool mEnable;  
  long mSpeedSamplingTime;
  long mSpeedLastMicros;
  long mSpeedLastCounter;
  long mSpeed;
};

#endif // __GG_QUADRATURE_DECODER__

