#pragma once

#include <WString.h>

template <typename TValueType>
String ToString(const TValueType& aValue) {
  return String(aValue);
}

template <typename TValueType>
TValueType FromString(const String& aString) {
  return TValueType(aString);
}
