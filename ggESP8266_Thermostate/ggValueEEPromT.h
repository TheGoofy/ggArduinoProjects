#pragma once

#include "ggValueEEProm.h"

template <class TValue>
class ggValueEEPromT : protected ggValueEEProm {

public:

  ggValueEEPromT()
  : ggValueEEProm(sizeof(TValue)),
    mValue(),
    mModified(true) {
  }

  ggValueEEPromT(const TValue& aValue)
  : ggValueEEProm(sizeof(TValue)),
    mValue(aValue),
    mModified(true) {
  }

  inline operator const TValue& () const {
    return mValue;
  }

  inline ggValueEEPromT& operator = (const ggValueEEPromT& aOther) {
    Set(aOther.mValue);
    return *this;
  }

  inline ggValueEEPromT& operator = (const TValue& aValue) {
    Set(aValue);
    return *this;
  }

  inline ggValueEEPromT& operator += (const TValue& aValue) {
    Set(mValue + aValue);
    return *this;
  }

  inline ggValueEEPromT& operator -= (const TValue& aValue) {
    Set(mValue - aValue);
    return *this;
  }

  template <typename TFactor>
  inline ggValueEEPromT& operator *= (const TFactor& aFactor) {
    Set(mValue * aFactor);
    return *this;
  }

  template <typename TDivisor>
  inline ggValueEEPromT& operator /= (const TDivisor& aDivisor) {
    Set(mValue / aDivisor);
    return *this;
  }

  inline const TValue& Get() const {
    return mValue;
  }

  inline void Set(const TValue& aValue) {
    if (mValue != aValue) {
      mValue = aValue;
      mModified = true;
      if (!WriteLazy()) Write(true);
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
    mModified = false;
    // Serial.printf("%s - mValue = ", __PRETTY_FUNCTION__);
    // Serial.println(mValue);
    // Serial.flush();
  }

  virtual void Write(bool aCommit) {
    EEPROM.put(mAddressEEProm, mValue);
    mModified = false;
    // Serial.printf("%s - mValue = ", __PRETTY_FUNCTION__);
    // Serial.println(mValue);
    // Serial.flush();
    if (aCommit) {
      WriteHeader();
      EEPROM.commit();
    }
  }

  TValue mValue;
  bool mModified;

};
