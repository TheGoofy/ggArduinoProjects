#pragma once

#include <WString.h>

template <typename TValue>
String ToString(const TValue& aValue) {
  return String(aValue);
}

template <typename TValue>
TValue FromString(const String& aString) {
  return TValue(aString);
}
