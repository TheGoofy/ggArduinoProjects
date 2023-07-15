#pragma once

#include <array>
#include <algorithm>
#include <random>

template <class TValue, std::size_t TSize>
class ggArrayShuffled : public std::array<TValue, TSize>
{
public:
  typedef std::array<TValue, TSize> tBase;
  ggArrayShuffled(const TValue& aValueBegin)
  : mRandomGenerator(mRandomDevice()) {
    std::iota(tBase::begin(), tBase::end(), aValueBegin);
  }
  void Shuffle() {
    // std::shuffle(tBase::begin(), tBase::end(), mRandomGenerator);
    std::shuffle(tBase::begin() + 0*TSize/2, tBase::begin() + 1*TSize/2, mRandomGenerator);
    std::shuffle(tBase::begin() + 1*TSize/2, tBase::begin() + 2*TSize/2, mRandomGenerator);
  }
private:
  std::random_device mRandomDevice;
  std::mt19937 mRandomGenerator;
};
