#pragma once

#include <limits>

#ifndef M_PI
#define M_PI   3.14159265358979323846 // Pi
#endif

#ifndef M_2PI
#define M_2PI  6.28318530717958647693 // 2 * Pi
#endif

#ifndef M_PI2
#define M_PI2  1.57079632679489661923 // 1/2 * Pi
#endif

#ifndef M_2PI3
#define M_2PI3 2.09439510239319549231 // 2/3 * Pi
#endif

template <class TValue = int16_t, uint8_t TPeriodBits = 8>
class ggLookupSinusoidT
{
public:

  ggLookupSinusoidT() {
    InitTable();
  }

  static constexpr TValue GetAmplitude() {
    return std::numeric_limits<TValue>::max();
  }

  static constexpr uint32_t GetPeriod() {
    return (uint32_t)1 << TPeriodBits;
  }

  static constexpr uint32_t GetPeriodMask() {
    return GetPeriod() - 1;
  }

  inline TValue Get(const int32_t aAngle) const {
    uint32_t vAngle = aAngle & GetPeriodMask();
    return mTable[vAngle];
  }

  inline TValue Get(const TValue aAmplitude, const int32_t aAngle) const {
    int64_t vValue = Get(aAngle);
    vValue *= aAmplitude;
    vValue += (vValue > 0) ? (GetAmplitude() / 2) : (- GetAmplitude() / 2);
    return vValue / GetAmplitude();
  }

  template <typename TSerial>
  void PrintT(TSerial& aSerial) const {
    aSerial.printf("%s\n", __PRETTY_FUNCTION__);
    aSerial.printf("Amplitude: %d\n", GetAmplitude());
    aSerial.printf("Period: 0x%04x (%d)\n", GetPeriod(), GetPeriod());
    for (uint32_t vAngle = 0; vAngle < GetSize(); vAngle++) {
      aSerial.printf("mTable[%d]: %d\n", vAngle, mTable[vAngle]);
    }
    // for (int32_t vAngle = -10; vAngle <= static_cast<int32_t>(GetPeriod()) + 10; vAngle++) {
    //   aSerial.printf("Get(%d): %d\n", vAngle, Get(100, vAngle));
    // }
  }

private:

  inline void InitTable() {
    for (uint32_t vIndex = 0; vIndex < GetSize(); vIndex++) {
      double vAlpha = M_2PI * (double)vIndex / (double)GetSize();
      vAlpha = M_2PI3 - abs(M_2PI3 - vAlpha);
      double vSin = GetAmplitude() * sin(vAlpha);
      mTable[vIndex] = vSin > 0 ? static_cast<TValue>(vSin + 0.5) : 0;
    }
  }

  static constexpr uint32_t GetSize() {
    return GetPeriod();
  }

  TValue mTable[GetSize()];
  
};
