#pragma once

#include "ggValueEEPromT.h"

template <int TSize = 32>
class ggValueEEPromString : public ggValueEEPromT<char[TSize]> {

public:

  typedef ggValueEEPromT<char[TSize]> tBase;

  ggValueEEPromString()
  : tBase(),
    mValueString("") {
    strncpy(tBase::mValue, mValueString.c_str(), TSize);
  }

  ggValueEEPromString(const String& aValue)
  : tBase(),
    mValueString(aValue) {
    strncpy(tBase::mValue, mValueString.c_str(), TSize);
  }

  inline ggValueEEPromString& operator = (const ggValueEEPromString& aOther) {
    Set(aOther.mValueString);
    return *this;
  }

  inline ggValueEEPromString& operator = (const String& aValue) {
    Set(aValue);
    return *this;
  }

  inline operator const String& () const {
    return Get();
  }

  void Set(const String& aValue) {
    if (mValueString != aValue) {
      mValueString = aValue;
      strncpy(tBase::mValue, mValueString.c_str(), TSize);
      tBase::mModified = true;
      if (!tBase::WriteLazy()) tBase::Write(true); // aCommit
    }
  }

  inline const String& Get() const {
    return mValueString;
  }

protected:

  virtual void Read() {
    tBase::Read();
    mValueString = String(tBase::mValue);
  }

private:

  String mValueString;
  
};
