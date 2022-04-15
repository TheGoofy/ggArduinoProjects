#pragma once

#include <Stream.h>

class ggNullStream : public Stream {

public:

  ggNullStream() {
  }

  // "Stream" overrides
  virtual int available() {
    return 0;
  }

  virtual int read() {
    return -1;
  }

  virtual int peek() {
    return -1;
  }

  virtual void flush() {
  };
  
  // "Print" overrides
  virtual size_t write(uint8_t aChar) {
    return 1;
  };

};
