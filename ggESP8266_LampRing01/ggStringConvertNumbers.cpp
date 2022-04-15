#include "ggStringConvertNumbers.h"


template <>
int FromString(const String& aString) {
  return aString.toInt();
}


template <>
float FromString(const String& aString) {
  return aString.toFloat();
}
