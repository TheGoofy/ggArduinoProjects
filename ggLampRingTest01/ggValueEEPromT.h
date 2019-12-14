#pragma once

#include "ggValueEEProm.h"

template <class TValueType>
class ggValueEEPromT : private ggValueEEProm {

public:

  ggValueEEPromT()
  : ggValueEEProm(sizeof(TValueType)) {
    ggValueEEProm::Value<TValueType>() = TValueType();
  }

  ggValueEEPromT(const TValueType& aDefaultValue)
  : ggValueEEProm(sizeof(TValueType)) {
    ggValueEEProm::Value<TValueType>() = aDefaultValue;
  }

  inline TValueType& Value() {
    return ggValueEEProm::Value<TValueType>();
  }

  inline const TValueType& Value() const {
    return ggValueEEProm::Value<TValueType>();
  }

  inline operator TValueType() {
    return Value();
  }

  inline TValueType& operator = (const TValueType& aValue) {
    Set(aValue);
    return Value();
  }

  inline const TValueType& Get() const {
    return ggValueEEProm::Value<TValueType>();
  }

  inline void Set(const TValueType& aValue) {
    if (ggValueEEProm::Value<TValueType>() != aValue) {
      ggValueEEProm::Value<TValueType>() = aValue;
      ggValueEEProm::Write<TValueType>();
    }
  }

  inline void Write() {
    ggValueEEProm::Write<TValueType>();
  }

};

