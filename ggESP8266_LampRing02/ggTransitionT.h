#pragma once

template <class TValue>
class ggTransitionT {

public:

  ggTransitionT() :
    mMicrosStart(0),
    mMicrosEnd(0),
    mMicrosDelta(0) {
  }

  // note that maximum is 35 minutes (2^31 micros)
  ggTransitionT(float aSeconds) :
    mMicrosStart(0),
    mMicrosEnd(0),
    mMicrosDelta(1000000.0f * aSeconds + 0.5f) {
  }

  // note that maximum is 35 minutes (2^31 micros)
  ggTransitionT(unsigned long aMicros) :
    mMicrosStart(0),
    mMicrosEnd(0),
    mMicrosDelta(aMicros) {
  }

  // note that maximum is 35 minutes (2^31 micros)
  void SetSeconds(float aSeconds) {
    SetMicros(1000000.0f * aSeconds + 0.5f);
  }

  // note that maximum is 35 minutes (2^31 micros)
  void SetMicros(unsigned long aMicros) {
    mMicrosDelta = aMicros;
    if (!Finished()) {
      mMicrosEnd = mMicrosStart + mMicrosDelta;
    }
  }

  float GetSeconds() const {
    return 0.000001f * mMicrosDelta;
  }

  unsigned long GetMicros() const {
    return mMicrosDelta;
  }

  void Set(const TValue& aValue) {
    if (aValue != mValueEnd) {
      mValueStart = Get();
      mValueEnd = aValue;
      mMicrosStart = micros();
      mMicrosEnd = mMicrosStart + mMicrosDelta;
    }
  }

  // returns the "current" interpolated value
  const TValue& Get() const {
    if (Finished()) {
      return mValueEnd;
    }
    unsigned long vMicros = micros();
    long vMicrosPassed = vMicros - mMicrosStart;
    long vMicrosRemain = mMicrosEnd - vMicros;
    if ((vMicrosPassed <= 0) || (vMicrosRemain <= 0)) {
      mMicrosStart = mMicrosEnd;
      return mValueEnd;
    }
    float vT = (float)vMicrosPassed / (float)mMicrosDelta;
    mValue = ggInterpolate<TValue>(mValueStart, mValueEnd, vT);
    return mValue;
  }

  inline const TValue& GetStart() const {
    return mValueStart;
  }

  inline const TValue& GetEnd() const {
    return mValueEnd;
  }

  inline bool Finished() const {
    return (mMicrosStart == mMicrosEnd) || (mValueStart == mValueEnd);
  }

  inline bool operator == (const TValue& aValue) const {
    return mValueEnd == aValue;
  }

  inline bool operator == (const ggTransitionT& aOther) const {
    return mValueEnd == aOther.mValueEnd;
  }

  inline bool operator != (const TValue& aValue) const {
    return mValueEnd != aValue;
  }

  inline bool operator != (const ggTransitionT& aOther) const {
    return mValueEnd != aOther.mValueEnd;
  }

  inline ggTransitionT& operator = (const TValue& aValue) {
    Set(aValue);
    return *this;
  }

  inline ggTransitionT& operator += (const TValue& aValue) {
    Set(mValueEnd + aValue);
    return *this;
  }

  inline ggTransitionT& operator -= (const TValue& aValue) {
    Set(mValueEnd - aValue);
    return *this;
  }

  inline operator const TValue& () const {
    return Get();
  }

private:

  TValue mValueStart;
  TValue mValueEnd;
  mutable TValue mValue;

  mutable unsigned long mMicrosStart;
  unsigned long mMicrosEnd;
  unsigned long mMicrosDelta;

};
