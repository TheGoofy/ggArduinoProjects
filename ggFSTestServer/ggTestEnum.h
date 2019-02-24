#pragma once

#include "ggStringConvertT.h"

struct ggTestEnum {
  enum tEnum {
    eEnumA = 0,
    eEnumB = 1,
    eEnumC = 2
  };
};

template <>
String ToString(const ggTestEnum::tEnum& aEnum) {
  switch (aEnum) {
    case ggTestEnum::eEnumA: return "eEnumA";
    case ggTestEnum::eEnumB: return "eEnumB";
    case ggTestEnum::eEnumC: return "eEnumC";
    default: return "eEnumUnknown";
  }
}

template <>
ggTestEnum::tEnum FromString(const String& aString) {
  if (aString == "eEnumA") return ggTestEnum::eEnumA;
  if (aString == "eEnumB") return ggTestEnum::eEnumB;
  if (aString == "eEnumC") return ggTestEnum::eEnumC;
  return ggTestEnum::eEnumA;
}
