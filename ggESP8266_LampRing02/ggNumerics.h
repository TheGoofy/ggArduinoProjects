#pragma once

#include <type_traits>
#include <limits>
#include <stdint.h>
#include <math.h>

template <typename T1, typename T2>
inline bool ggIsLess(const T1& aValue1, const T2& aValue2) {
  typedef typename std::common_type<T1,T2>::type tCommonType;
  return static_cast<tCommonType>(aValue1) < static_cast<tCommonType>(aValue2);
}

template <typename T1, typename T2>
inline T1 ggClamp(const T2& aValue, const T1& aValueMin, const T1& aValueMax) {
  if (ggIsLess(aValue, aValueMin)) return aValueMin;
  if (ggIsLess(aValueMax, aValue)) return aValueMax;
  return static_cast<T1>(aValue);
}

template <typename T1, typename T2>
inline T1 ggClampNumericLimits(const T2& aValue) {
  return ggClamp(aValue, std::numeric_limits<T1>::lowest(), std::numeric_limits<T1>::max());
}

template <typename T>
inline const T& ggClamp(const T& aValue, const T& aValueMin, const T& aValueMax) {
  return (aValue < aValueMin) ? aValueMin : ((aValue > aValueMax) ? aValueMax : aValue);
}

template <typename T>
inline const T& ggMin(const T& aA, const T& aB) {
  return (aA < aB) ? aA : aB;
}

template <typename T>
inline const T& ggMax(const T& aA, const T& aB) {
  return (aA < aB) ? aB : aA;
}

template <typename T>
inline const T& ggMin(const T& aA, const T& aB, const T& aC) {
  if (aA < aB) return ggMin(aA, aC);
  else return ggMin(aB, aC);
}

template <typename T>
inline const T& ggMax(const T& aA, const T& aB, const T& aC) {
  if (aA < aB) return ggMax(aB, aC);
  else return ggMax(aA, aC);
}

template <typename T>
inline T ggAbs(const T& aValue) {
  return aValue > 0 ? aValue : -aValue;
}

template <typename T>
inline bool ggEqual(const T& aA, const T& aB) {
  return aA == aB;
}

template <typename T>
inline bool ggEqual(const T& aA, const T& aB, int aSignificantDigits) {
  if (ggAbs(aA) > 0) {
    const T vRatio = aA / aB;
    const double vEps = 1.0 / pow(10.0, aSignificantDigits);
    const T vLimitMin = static_cast<T>(1.0 - vEps);
    const T vLimitMax = static_cast<T>(1.0 + vEps);
    return (vLimitMin < vRatio) && (vRatio < vLimitMax);
  }
  // aA is zero. if aB is zero too, they aA and aB are equal (both zero)
  return ggAbs(aB) <= 0; // trick to avoid "==" warning (unsafe float compare)
}

template <>
inline bool ggEqual(const float& aA, const float& aB) {
  return ggEqual(aA, aB, 3);
}

template <>
inline bool ggEqual(const double& aA, const double& aB) {
  return ggEqual(aA, aB, 6);
}

template <typename TOut, typename TIn>
TOut ggRound(const TIn aValue) {
  return ggClampNumericLimits<TOut>(aValue);
}

template <typename TOut>
TOut ggRound(const float aValue) {
  if (std::is_floating_point<TOut>()) return static_cast<TOut>(aValue);
  else return ggClampNumericLimits<TOut>(static_cast<int64_t>(aValue >= 0 ? aValue + 0.5f : aValue - 0.5f));
}

template <typename TOut>
TOut ggRound(const double aValue) {
  if (std::is_floating_point<TOut>()) return static_cast<TOut>(aValue);
  else return ggClampNumericLimits<TOut>(static_cast<int64_t>(aValue >= 0 ? aValue + 0.5 : aValue - 0.5));
}

float ggRoundToSD(float aValue, int aSignificantDigits = 3);

template <typename T>
inline T ggInterpolate(const T& aA, const T& aB, float aT) {
  return aA + aT * (aB - aA);
}
