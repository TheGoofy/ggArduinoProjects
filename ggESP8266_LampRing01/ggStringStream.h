#pragma once

#include <Stream.h>

class ggStringStream : public Stream {

public:

  ggStringStream(String &aString)
  : mString(aString),
    mPosition(0) {
  }

  // "Stream" overrides
  virtual int available() {
    return mString.length() - mPosition;
  }

  virtual int read() {
    return mPosition < mString.length() ? mString[mPosition++] : -1;
  }

  virtual int peek() {
    return mPosition < mString.length() ? mString[mPosition] : -1;
  }

  virtual void flush() {
  };
  
  // "Print" overrides
  virtual size_t write(uint8_t aChar) {
    mString += (char)aChar;
    return 1;
  };

private:

  String& mString;
  unsigned int mLength;
  unsigned int mPosition;

};
