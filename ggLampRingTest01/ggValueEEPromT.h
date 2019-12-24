#pragma once

#include "ggValueEEProm.h"

template <class TValueType>
class ggValueEEPromT : private ggValueEEProm {

public:

  ggValueEEPromT()
  : ggValueEEProm(sizeof(TValueType)),
    mValue() {
  }

  ggValueEEPromT(const TValueType& aValue)
  : ggValueEEProm(sizeof(TValueType)),
    mValue(aValue) {
  }

  virtual int GetSize() const {
    return sizeof(TValueType);
  }
  
  virtual void* GetValuePtr() {
    return &mValue;
  }

  inline operator const TValueType& () const {
    return mValue;
  }

  inline ggValueEEPromT& operator = (const TValueType& aValue) {
    Set(aValue);
    return *this;
  }

  inline const TValueType& Get() const {
    return mValue;
  }

  inline void Set(const TValueType& aValue) {
    if (mValue != aValue) {
      mValue = aValue;
      Write(true);
    }
  }

private:

  virtual void Read() {
    EEPROM.get(mAddressEEProm, mValue);
    Serial.printf("%s - mValue = ", __PRETTY_FUNCTION__);
    // Serial.println(mValue);
    Serial.flush();
  }
  
  virtual void Write(bool aCommit) {
    EEPROM.put(mAddressEEProm, mValue);
    Serial.printf("%s - mValue = ", __PRETTY_FUNCTION__);
    // Serial.println(mValue);
    Serial.flush();
    if (aCommit) {
      WriteHeader();
      EEPROM.commit();
    }
  }

  TValueType mValue;

};
