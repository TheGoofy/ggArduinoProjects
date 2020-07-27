#pragma once

template <typename TValue>
String ToJson(const TValue& aValue) {
  return String(aValue);
}

template <typename TValue>
TValue FromJson(const String& aString) {
  return TValue(aString);
}
