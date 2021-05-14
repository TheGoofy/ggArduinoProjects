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

  void Set(bool aOnA, bool aOnB, bool aOnOnBoard) {
    SetA(aOnA);
    SetB(aOnB);
    SetOnBoard(aOnOnBoard);
  }

  bool GetA() const {
    return digitalRead(mPinLedA);
  }

  bool GetB() const {
    return digitalRead(mPinLedB);
  }

  bool GetOnBoard() const {
    return digitalRead(mPinLedOnBoard);
  }

private:

  const int mPinLedA;
  const int mPinLedB;
  const int mPinLedOnBoard;
  
};
