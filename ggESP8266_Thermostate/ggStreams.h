#pragma once

#include <Stream.h>
#include <algorithm>
#include <vector>

class ggStreams : public Stream, public std::list<Stream*> {

public:

  ggStreams() {
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
    std::for_each(begin(), end(), [] (Stream* aStream) {
      aStream->flush();
    });
  };

  // "Print" overrides
  virtual size_t write(uint8_t aChar) {
    std::for_each(begin(), end(), [&aChar] (Stream* aStream) {
      aStream->write(aChar);
    });
    return 1;
  };

};
