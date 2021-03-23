#pragma once

class ggStatusLEDs {
  
public:

  ggStatusLEDs(
    int aPinLedA,
    int aPinLedB,
    int aPinLedOnBoard)
  : mPinLedA(aPinLedA),
    mPinLedB(aPinLedB),
    mPinLedOnBoard(aPinLedOnBoard) {
  }

  void Begin() {
    pinMode(mPinLedA, OUTPUT);
    pinMode(mPinLedB, OUTPUT);
    pinMode(mPinLedOnBoard, OUTPUT);
    SetA(false);
    SetB(false);
    SetOnBoard(false);
  }

  void SetA(bool aOn) {
    digitalWrite(mPinLedA, aOn);
  }

  void SetB(bool aOn) {
    digitalWrite(mPinLedB, aOn);
  }

  void SetOnBoard(bool aOn) {
    digitalWrite(mPinLedOnBoard, aOn);
  }

private:

  int mPinLedA;
  int mPinLedB;
  int mPinLedOnBoard;
  
};
