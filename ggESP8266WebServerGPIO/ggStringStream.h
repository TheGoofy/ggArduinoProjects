#pragma once

#include <Stream.h>

class ggStringStream : public Stream {

public:

  explicit ggStringStream(String& aString)
  : Stream(),
    mString(aString),
    mPosition(0) {
  }

  // (override from Write)
  virtual size_t write(uint8_t aChar) {
    mString += (char)aChar;
    return 1;
  }

  // (override from Stream)
  virtual int available() {
    return mString.length() - mPosition;
  }
  
  // (override from Stream)
  virtual int read() {
    return mPosition < mString.length() ? mString[mPosition++] : -1;    
  }
  
  // (override from Stream)
  virtual int peek() {
    return mPosition < mString.length() ? mString[mPosition] : -1;
  }
  
private:

  String& mString;
  int mPosition;  
  
};

