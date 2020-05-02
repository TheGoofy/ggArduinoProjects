#pragma once

#include "ggValueEEProm.h"

template <class TValue>
class ggValueEEPromT : private ggValueEEProm {

public:

  ggValueEEPromT()
  : ggValueEEProm(sizeof(TValue)),
    mValue() {
  }

  ggValueEEPromT(const TValue& aValue)
  : ggValueEEProm(sizeof(TValue)),
    mValue(aValue) {
  }

  inline operator const TValue& () const {
    return mValue;
  }

  inline ggValueEEPromT& operator = (const TValue& aValue) {
    Set(aValue);
    return *this;
  }

  inline const TValue& Get() const {
    return mValue;
  }

  inline void Set(const TValue& aValue) {
    if (mValue != aValue) {
      mValue = aValue;
      Write(true);
    }
  }

protected:

  virtual int GetSize() const {
    return sizeof(TValue);
  }
  
  virtual void* GetValuePtr() {
    return &mValue;
  }

  virtual void Read() {
    EEPROM.get(mAddressEEProm, mValue);
    // Serial.printf("%s - mValue = ", __PRETTY_FUNCTION__);
    // Serial.println(mValue);
    // Serial.flush();
  }

  virtual void Write(bool aCommit) {
    EEPROM.put(mAddressEEProm, mValue);
    // Serial.printf("%s - mValue = ", __PRETTY_FUNCTION__);
    // Serial.println(mValue);
    // Serial.flush();
    if (aCommit) {
      WriteHeader();
      EEPROM.commit();
    }
  }

  TValue mValue;

};
