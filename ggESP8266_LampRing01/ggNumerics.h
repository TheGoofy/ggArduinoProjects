#pragma once

template <typename T>
inline T ggAbs(const T& aValue) {
  return aValue > 0 ? aValue : -aValue;
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
