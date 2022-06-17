#pragma once

#define GG_MORSE_SUPPORT_NUMBERS
#define GG_MORSE_SUPPORT_LETTERS
#define GG_MORSE_SUPPORT_SYMBOLS

class ggMorse
{
public:

  typedef void (*tSignalFunc)(long aMillis);
  typedef void (*tSpaceFunc)(long aMillis, bool aIsSymbol);

  ggMorse(tSignalFunc aSignalFunc,
          tSpaceFunc aSpaceFunc,
          long aMillisDit = 200);

  void Signal(const char* aText) const;

private:
  
  void SignalChar(const char aChar) const;

  tSignalFunc mSignalFunc;
  tSpaceFunc mSpaceFunc;

  const long mMillisDit;

};
