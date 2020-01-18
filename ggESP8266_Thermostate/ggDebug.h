#pragma once

#include <Arduino.h>

#define GG_DEBUG() \
  ggDebug vDebug(FPSTR(__PRETTY_FUNCTION__));

class ggDebug {

public:

  ggDebug(const String& aName)
  : mName(aName) {
    Begin();
    mDepth++;
  }

  ggDebug(const String& aName, const String& aAttributes)
  : mName(aName) {
    aAttributes != "" ? Begin(aAttributes) : Begin();
    mDepth++;
  }

  virtual ~ggDebug() {
    mDepth--;
    End();
  }

  template <typename... TArguments>
  void PrintF(const char* aFormat, TArguments... aArguments) {
    if (GetEnable()) {
      GetStream().print(Indent().c_str());
      GetStream().printf(aFormat, aArguments...);
      GetStream().flush();
    }
  }
 
  static void SetEnable(bool aEnable) {
    mEnable = aEnable;
  }

  static bool GetEnable() {
    return mEnable;
  }

  static void SetStream(Stream& aStream) {
    mStream = &aStream;
  }

  static Stream& GetStream() {
    if (mStream == nullptr) mStream = &Serial;
    return *mStream;
  }

private:

  void Begin() {
    if (GetEnable()) {
      GetStream().printf("%s<%s>\n", Indent().c_str(), mName.c_str());
      GetStream().flush();
    }
  }

  void Begin(const String& aAttributes) {
    if (GetEnable()) {
      GetStream().printf("%s<%s %s>\n", Indent().c_str(), mName.c_str(), aAttributes.c_str());
      GetStream().flush();
    }
  }

  void End() {
    if (GetEnable()) {
      GetStream().printf("%s</%s>\n", Indent().c_str(), mName.c_str());
      GetStream().flush();
    }
  }

  String Indent() const {
    String vIndent;
    for (unsigned int vDepth = 0; vDepth < mDepth; vDepth++) vIndent += "  ";
    return vIndent;
  }

  const String mName;

  static bool mEnable;
  static Stream* mStream;
  static unsigned int mDepth;
  
};
