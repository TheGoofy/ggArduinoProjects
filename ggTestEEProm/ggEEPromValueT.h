#pragma once

#include "ggEEPromValue.h"

template <class TValueType>
class ggEEPromValueT : private ggEEPromValue {

public:

  ggEEPromValueT()
  : ggEEPromValue(sizeof(TValueType)) {
    ggEEPromValue::Value<TValueType>() = TValueType();
  }

  ggEEPromValueT(const TValueType& aDefaultValue)
  : ggEEPromValue(sizeof(TValueType)) {
    ggEEPromValue::Value<TValueType>() = aDefaultValue;
  }

  inline TValueType& Value() {
    return ggEEPromValue::Value<TValueType>();
  }

  inline const TValueType& Value() const {
    return ggEEPromValue::Value<TValueType>();
  }

  inline operator TValueType() {
    return Value();
  }

  inline TValueType& operator = (const TValueType& aValue) {
    Set(aValue);
    return Value();
  }

  inline void Set(const TValueType& aValue) {
    if (ggEEPromValue::Value<TValueType>() != aValue) {
      ggEEPromValue::Value<TValueType>() = aValue;
      ggEEPromValue::Write<TValueType>();
    }
  }

  inline void Write() {
    ggEEPromValue::Write<TValueType>();
  }

};

