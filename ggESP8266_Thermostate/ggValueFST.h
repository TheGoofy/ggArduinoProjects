#pragma once

#include <FS.h>

#include "ggStringConvertT.h"

template <class TValueType>
class ggValueFST {

public:

  ggValueFST(const String& aValueName,
             const TValueType& aDefaultValue)
  : mValueName(aValueName),
    mValue(aDefaultValue) {
  }

  const TValueType& Get() const {
    return mValue;
  }

  void Set(const TValueType& aValue) {
    if (mValue != aValue) {
      mValue = aValue;
      WriteFile();
    }
  }

  void Begin() {
    SPIFFS.begin();
    ReadFile();
  }

private:

  void ReadFile() {
    File vFile = SPIFFS.open(mValueName, "r");
    if (vFile) {
      String vString = "";
      while (vFile.available()) vString += char(vFile.read());
      mValue = FromString<TValueType>(vString);
      vFile.close();
    }
  }

  void WriteFile() {
    File vFile = SPIFFS.open(mValueName, "w");
    if (vFile) {
      String vString = ToString<TValueType>(mValue);
      vFile.print(vString);
      vFile.close();
    }
  }

  const String mValueName;
  TValueType mValue;
  
};
