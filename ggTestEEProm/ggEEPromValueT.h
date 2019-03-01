#pragma once

#include "ggEEPromValue.h"

template <class TValueType>
class ggEEPromValueT : private ggEEPromValue {

public:

  ggEEPromValueT(const TValueType& aDefaultValue)
  : ggEEPromValue(sizeof(TValueType)) {
    ggEEPromValue::Value<TValueType>() = aDefaultValue;
  }

  inline const TValueType& Get() const {
    return Value<TValueType>();
  }

  inline void Set(const TValueType& aValue) {
    if (Value<TValueType>() != aValue) {
      Value<TValueType>() = aValue;
      Write(aValue);
    }
  }

};

