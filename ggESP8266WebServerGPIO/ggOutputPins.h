#pragma once

namespace ggOutputPins {

  const int mSize = 4;

  const int mPins[mSize] = {
    0, // GPIO0
    1, // GPIO1, TX
    2, // GPIO2
    3  // GPIO3, RX
  };

  template <typename TFunction>
  void ForEach(TFunction aFunction) {
    for (int vIndex = 0; vIndex < mSize; vIndex++) {
      aFunction(vIndex);
    }
  }

  template <typename TFunction>
  void ForEachPin(TFunction aFunction) {
    ForEach([&] (int aIndex) {
      aFunction(mPins[aIndex]);
    });
  }
  
  void Begin() {
    ForEachPin([] (int aPin) {
      pinMode(aPin, OUTPUT);
    });
  }

  int GetSize() {
    return mSize;
  }

  void Set(int aIndex, bool aHigh) {
    digitalWrite(mPins[aIndex], aHigh);
  }

  bool Get(int aIndex) {
    return digitalRead(mPins[aIndex]);
  }

  void Print(Stream& aStream) {
    ForEach([&] (int aIndex) {
      aStream.printf("Pin[%d] = %d\n", aIndex, Get(aIndex));
    });
  }

};

