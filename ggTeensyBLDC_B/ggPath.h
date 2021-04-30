#pragma once

class ggPath
{
public:

  ggPath()
  : mSMin(0.0), mSMax(1.0), mSRollOver(false), mAMax(1.0), mBMax(1.0), // limits
    mT0(0), mT1(0), mS0(0.0), mS1(0.0), mV0(0.0), mV1(0.0), // trajectory
    mMicros(micros()), mT(0), mS(0.0), mV(0.0), mA(0.0) // current
  {
  }

  void SetLimits(float aSMin, float aSMax, float aAMax, float aBMax) {
    mSMin = aSMin;
    mSMax = aSMax;
    mAMax = aAMax > 0.0 ? aAMax : -aAMax;
    mBMax = aBMax > 0.0 ? aBMax : -aBMax;
  }

  void SetRollOverS(bool aSRollOver) {
    mSRollOver = aSRollOver;
  }

  void MoveToV(float aV, float aA) {
    // nothing to do if final speed doesn't change
    if (aV == mV1) return;
    // calculate current trajectory point
    Update();
    // use current trajectory point for "p0"
    mT0 = mT;
    mS0 = mS;
    mV0 = mV;
    // calculate trajectory point "p1"
    float vVDelta = aV - mV0;
    mA = vVDelta > 0.0 ? aA : -aA;
    float vTDelta = vVDelta / mA;
    mT1 = mT0 + static_cast<uint64_t>(1000000.0 * vTDelta + 0.5);
    mS1 = mS0 + (mV0 + 0.5 * mA * vTDelta) * vTDelta;
    mV1 = aV;
  }

  inline float GetT() const {
    return 0.000001 * mT;
  }

  inline float GetS() const {
    return mS;
  }

  inline float GetV() const {
    return mV;
  }

  inline float GetA() const {
    return mA;
  }

  void Update() {
    
    // update time (handle time rollover)
    unsigned long vMicros = micros();
    unsigned long vMicrosDelta = vMicros - mMicros;
    mT += vMicrosDelta;
    mMicros = vMicros;
    
    // check trajectory phase
    if (mT < mT1) {
      // acceleration phase
      float vTDelta = 0.000001 * (mT - mT0);
      mS = mS0 + (mV0 + 0.5 * mA * vTDelta) * vTDelta;
      mV = mV0 + mA * vTDelta;
    }
    else {
      // constant speed phase
      float vTDelta = 0.000001 * (mT - mT1);
      mS = mS1 + mV1 * vTDelta;
      mV = mV1;
      mA = 0.0;
    }
  }

private:

  // limits
  float mSMin;
  float mSMax;
  bool mSRollOver;
  float mAMax;
  float mBMax;

  // trajectory points
  uint64_t mT0;
  uint64_t mT1;
  float mS0;
  float mS1;
  float mV0;
  float mV1;

  // last time: rolls over after 72 minutes => helper for "mT"
  unsigned long mMicros;

  // current values
  uint64_t mT; // time [microseconds]
  float mS; // position
  float mV; // speed
  float mA; // acceleration

};
