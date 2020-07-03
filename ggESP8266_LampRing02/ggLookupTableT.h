#pragma once

#include <functional>

template <class TValue, uint32_t TSize = 101>
class ggLookupTableT
{
public:

  static_assert(TSize > 1, "lookuptable size is too small");

  typedef std::function<TValue (const TValue& aInput)> tFunc;

  ggLookupTableT(const TValue& aInputMin,
                 const TValue& aInputMax,
                 tFunc aFunc)
  : mInputMin(aInputMin),
    mInputMax(aInputMax),
    mInputRange(aInputMax - aInputMin),
    mOutputFunc(aFunc)
  {
    Init();
  }

  inline uint32_t GetSize() const {
    return TSize;
  }

  inline const TValue& GetInputMin() const {
    return mInputMin;
  }

  inline const TValue& GetInputMax() const {
    return mInputMax;
  }

  inline TValue GetExact(const TValue& aInput) const {
    return mOutputFunc(aInput);
  }

  inline TValue GetNN(const TValue& aInput) const {
    float vInputIndexF = (aInput - mInputMin) * (TSize - 1) / mInputRange;
    int32_t vInputIndex = ggRound<int32_t>(vInputIndexF);
    return mOutputs[ggClamp<int32_t>(vInputIndex, 0, TSize - 1)];
  }

  inline TValue Get(const TValue& aInput) const {
    float vInputIndex = (aInput - mInputMin) * (TSize - 1) / mInputRange;
    int32_t vInputIndexA = floor(vInputIndex);
    if (vInputIndexA < 0) return mOutputs.front();
    int32_t vInputIndexB = vInputIndexA + 1;
    if (vInputIndexB >= TSize) return mOutputs.back();
    float vInputIndexT = vInputIndex - vInputIndexA;
    const TValue& vOutputA = mOutputs[vInputIndexA];
    const TValue& vOutputB = mOutputs[vInputIndexB];
    return  vOutputA + vInputIndexT * (vOutputB - vOutputA);;
  }

  inline TValue operator () (const TValue& aInput) const {
    return Get(aInput);
  }

private:

  void Init() {
    uint32_t vInputIndex = 0;
    for (auto& vOutput : mOutputs) {
      TValue vInput = mInputMin + mInputRange * vInputIndex / (TSize - 1);
      vOutput = mOutputFunc(vInput);
      vInputIndex++;
    }
  }

  const TValue mInputMin;
  const TValue mInputMax;
  const TValue mInputRange;
  tFunc mOutputFunc;

  std::array<TValue, TSize> mOutputs;

};
