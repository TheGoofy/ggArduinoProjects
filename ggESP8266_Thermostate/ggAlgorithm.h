#pragma once

template <typename T>
inline T ggAbs(const T& aValue) {
  return aValue > 0 ? aValue : -aValue;
}

template <typename T>
const T& ggClamp(const T& aValue, const T& aValueMin, const T& aValueMax) {
  return (aValue < aValueMin ? aValueMin : (aValueMax < aValue ? aValueMax : aValue));
}

float ggRoundToSD(float aValue, int aSignificantDigits = 3);

