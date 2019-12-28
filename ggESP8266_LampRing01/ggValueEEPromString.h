#pragma once

#include "ggValueEEPromT.h"

template <int TSize = 32>
class ggValueEEPromString : ggValueEEPromT<char[TSize]> {

public:

  typedef ggValueEEPromT<char[TSize]> tBase;

  ggValueEEPromString()
  : mValueString("") {
    strncpy(tBase::mValue, mValueString.c_str(), TSize);
  }

  ggValueEEPromString(const String& aValue)
  : mValueString(aValue) {
    strncpy(tBase::mValue, mValueString.c_str(), TSize);
  }

  void Set(const String& aValue) {
    if (mValueString != aValue) {
      mValueString = aValue;
      strncpy(tBase::mValue, mValueString.c_str(), TSize);
      tBase::Write(true); // aCommit
    }
  }

  const String& Get() const {
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
