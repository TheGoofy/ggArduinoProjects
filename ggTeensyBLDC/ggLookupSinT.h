#pragma once

#include <limits>

template <class TValue = int16_t, uint8_t TQuarterPeriodBits = 8>
class ggLookupSinT
{
public:

  ggLookupSinT() {
    InitTable();
  }

  static constexpr TValue GetAmplitude() {
    return std::numeric_limits<TValue>::max();
  }

  static constexpr uint32_t GetPeriod() {
    return 4 * ((uint32_t)1 << TQuarterPeriodBits);
  }

  inline TValue Get(const int32_t aAngle) const {
    uint16_t vAngle = aAngle & (GetPeriod() - 1);
    if (vAngle <= GetPeriod() / 2) {
      if (vAngle < GetPeriod() / 4) return mSin[vAngle];
      else return mSin[GetPeriod() / 2 - vAngle];
    }
    else {
      vAngle = vAngle & (GetPeriod() / 2 - 1);
      if (vAngle < GetPeriod() / 4) return - mSin[vAngle];
      else return - mSin[GetPeriod() / 2 - vAngle];
    }
    return mSin[vAngle];
  }

  inline TValue Get(const TValue aAmplitude, const int32_t aAngle) const {
    int64_t vValue = Get(aAngle);
    vValue *= aAmplitude;
    vValue += (vValue > 0) ? (GetAmplitude() / 2) : (- GetAmplitude() / 2);
    return vValue / GetAmplitude();
  }

  // float Get(float aAngle) const;

  template <typename TSerial>
  void PrintT(TSerial& aSerial) const {
    aSerial.printf("%s\n", __PRETTY_FUNCTION__);
    aSerial.printf("Amplitude: %d\n", GetAmplitude());
    aSerial.printf("Period: 0x%04x (%d)\n", GetPeriod(), GetPeriod());
    for (uint32_t vAngle = 0; vAngle < GetSize(); vAngle++) {
      aSerial.printf("mSin[%d]: %d\n", vAngle, mSin[vAngle]);
    }
    // for (int32_t vAngle = -10; vAngle <= static_cast<int32_t>(GetPeriod()) + 10; vAngle++) {
    //   aSerial.printf("Get(%d): %d\n", vAngle, Get(2, vAngle));
    // }
  }

private:

  inline void InitTable() {
    for (uint32_t vAngle = 0; vAngle < GetSize(); vAngle++) {
      mSin[vAngle] = static_cast<TValue>(GetAmplitude() * sin(vAngle * 6.2831853f / GetPeriod()) + 0.5f);
    }
  }

  static constexpr uint32_t GetSize() {
    return GetPeriod() / 4 + 1;
  }

  TValue mSin[GetSize()];
  
};
