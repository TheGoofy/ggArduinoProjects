#pragma once

template <typename T>
const T& ggClamp(const T& aValue, const T& aValueMin, const T& aValueMax) {
  return (aValue < aValueMin ? aValueMin : (aValueMax < aValue ? aValueMax : aValue));
}

