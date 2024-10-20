#pragma once

#define GG_MORSE_CODE_NA 0b11111111 // no code available

#define GG_MORSE_CODE_LENGTH_1 0b00000000 // 000----x (x can be 0:dot or 1:dash)
#define GG_MORSE_CODE_LENGTH_2 0b00100000 // 001---xx (x can be 0:dot or 1:dash)
#define GG_MORSE_CODE_LENGTH_3 0b01000000 // 010--xxx (x can be 0:dot or 1:dash)
#define GG_MORSE_CODE_LENGTH_4 0b01100000 // 011-xxxx (x can be 0:dot or 1:dash)
#define GG_MORSE_CODE_LENGTH_5 0b10000000 // 10-xxxxx (x can be 0:dot or 1:dash)
#define GG_MORSE_CODE_LENGTH_6 0b11000000 // 11xxxxxx (x can be 0:dot or 1:dash)

// morse-codes can be well mapped to first part of ascii-table
const PROGMEM uint8_t ggMorseCodeTable[95-32+1] = {
  GG_MORSE_CODE_NA,                  // 32 - (space)
  GG_MORSE_CODE_LENGTH_6 | 0b101011, // 33 - !
  GG_MORSE_CODE_LENGTH_6 | 0b010010, // 34 - "
  GG_MORSE_CODE_NA,                  // 35 - #
  GG_MORSE_CODE_NA,                  // 36 - $ (code is actually too long ...-..-)
  GG_MORSE_CODE_NA,                  // 37 - %
  GG_MORSE_CODE_LENGTH_5 |  0b01000, // 38 - &
  GG_MORSE_CODE_LENGTH_6 | 0b011110, // 39 - '
  GG_MORSE_CODE_LENGTH_5 |  0b10110, // 40 - (
  GG_MORSE_CODE_LENGTH_6 | 0b101101, // 41 - )
  GG_MORSE_CODE_NA,                  // 42 - *
  GG_MORSE_CODE_LENGTH_5 |  0b01010, // 43 - +
  GG_MORSE_CODE_LENGTH_6 | 0b110011, // 44 - ,
  GG_MORSE_CODE_LENGTH_6 | 0b100001, // 45 - -
  GG_MORSE_CODE_LENGTH_6 | 0b010101, // 46 - .
  GG_MORSE_CODE_LENGTH_5 |  0b10010, // 47 - /
  GG_MORSE_CODE_LENGTH_5 |  0b11111, // 48 - 0
  GG_MORSE_CODE_LENGTH_5 |  0b01111, // 49 - 1
  GG_MORSE_CODE_LENGTH_5 |  0b00111, // 50 - 2
  GG_MORSE_CODE_LENGTH_5 |  0b00011, // 51 - 3
  GG_MORSE_CODE_LENGTH_5 |  0b00001, // 52 - 4
  GG_MORSE_CODE_LENGTH_5 |  0b00000, // 53 - 5
  GG_MORSE_CODE_LENGTH_5 |  0b10000, // 54 - 6
  GG_MORSE_CODE_LENGTH_5 |  0b11000, // 55 - 7
  GG_MORSE_CODE_LENGTH_5 |  0b11100, // 56 - 8
  GG_MORSE_CODE_LENGTH_5 |  0b11110, // 57 - 9
  GG_MORSE_CODE_LENGTH_6 | 0b111000, // 58 - :
  GG_MORSE_CODE_LENGTH_6 | 0b101010, // 59 - ;
  GG_MORSE_CODE_NA,                  // 60 - <
  GG_MORSE_CODE_LENGTH_5 |  0b10001, // 61 - =
  GG_MORSE_CODE_NA,                  // 62 - >
  GG_MORSE_CODE_LENGTH_6 | 0b001100, // 63 - ?
  GG_MORSE_CODE_LENGTH_6 | 0b011010, // 64 - @
  GG_MORSE_CODE_LENGTH_2 |     0b01, // 65 - A
  GG_MORSE_CODE_LENGTH_4 |   0b1000, // 66 - B
  GG_MORSE_CODE_LENGTH_4 |   0b1010, // 67 - C
  GG_MORSE_CODE_LENGTH_3 |    0b100, // 68 - D
  GG_MORSE_CODE_LENGTH_1 |      0b0, // 69 - E
  GG_MORSE_CODE_LENGTH_4 |   0b0010, // 70 - F
  GG_MORSE_CODE_LENGTH_3 |    0b110, // 71 - G
  GG_MORSE_CODE_LENGTH_4 |   0b0000, // 72 - H
  GG_MORSE_CODE_LENGTH_2 |     0b00, // 73 - I
  GG_MORSE_CODE_LENGTH_4 |   0b0111, // 74 - J
  GG_MORSE_CODE_LENGTH_3 |    0b101, // 75 - K
  GG_MORSE_CODE_LENGTH_4 |   0b0100, // 76 - L
  GG_MORSE_CODE_LENGTH_2 |     0b11, // 77 - M
  GG_MORSE_CODE_LENGTH_2 |     0b10, // 78 - N
  GG_MORSE_CODE_LENGTH_3 |    0b111, // 79 - O
  GG_MORSE_CODE_LENGTH_4 |   0b0110, // 80 - P
  GG_MORSE_CODE_LENGTH_4 |   0b1101, // 81 - Q
  GG_MORSE_CODE_LENGTH_3 |    0b010, // 82 - R
  GG_MORSE_CODE_LENGTH_3 |    0b000, // 83 - S
  GG_MORSE_CODE_LENGTH_1 |      0b1, // 84 - T
  GG_MORSE_CODE_LENGTH_3 |    0b001, // 85 - U
  GG_MORSE_CODE_LENGTH_4 |   0b0001, // 86 - V
  GG_MORSE_CODE_LENGTH_3 |    0b011, // 87 - W
  GG_MORSE_CODE_LENGTH_4 |   0b1001, // 88 - X
  GG_MORSE_CODE_LENGTH_4 |   0b1011, // 89 - Y
  GG_MORSE_CODE_LENGTH_4 |   0b1100, // 90 - Z
  GG_MORSE_CODE_NA,                  // 91 - [
  GG_MORSE_CODE_NA,                  // 92 - (backslash)
  GG_MORSE_CODE_NA,                  // 93 - ]
  GG_MORSE_CODE_NA,                  // 94 - ^
  GG_MORSE_CODE_LENGTH_6 | 0b001101  // 95 - _
};


typedef void (*ggMorseSignalOnFunc)(uint8_t aNumDits);
typedef void (*ggMorseSignalOffFunc)(uint8_t aNumDits);

template<ggMorseSignalOnFunc TSignalOnFunc, // callback for signal "on"
         ggMorseSignalOffFunc TSignalOffFunc, // callback for signal "off"
         uint8_t TDashDits = 3, // duration od dash (a "Dit" is the shortest signal => duration of dot)
         uint8_t TSymbolPauseDits = 1, // pause wetween dots or dashes
         uint8_t TCharPauseDits = 3, // pause between chars
         uint8_t TWordPauseDits = 7> // pause between words (basically the duration of "space" char)

class ggMorseT
{
public:

  static constexpr uint8_t GetDotDits() {
    return 1;
  }

  static constexpr uint8_t GetDashDits() {
    return TDashDits;
  }

  static constexpr uint8_t GetSymbolPauseDits() {
    return TSymbolPauseDits;
  }

  static constexpr uint8_t GetCharPauseDits() {
    return TCharPauseDits;
  }

  static constexpr uint8_t GetWordPauseDits() {
    return TWordPauseDits;
  }

  static uint8_t GetCode(char aChar) {
    // conversion from lower to upper case
    aChar = ('a' <= aChar && aChar <= 'z') ? aChar - ('a' - 'A') : aChar;
    // read code from program memory table (if in range)
    return (32 <= aChar && aChar <= 95) ? pgm_read_byte(&ggMorseCodeTable[aChar - 32]) : GG_MORSE_CODE_NA;
  }

  static constexpr uint8_t GetCodeLength(uint8_t aCode) {
    return (aCode == GG_MORSE_CODE_NA) ? 0 : ((aCode & 0b10000000) ? (aCode >> 6) + 3 : (aCode >> 5) + 1);
  }

  static void SignalChar(const char aChar) {
    const uint8_t vCode = GetCode(aChar);
    uint8_t vLength = GetCodeLength(vCode);
    uint8_t vCodeBitMask = (1 << vLength) >> 1;
    while (vCodeBitMask) {
      TSignalOnFunc(vCode & vCodeBitMask ? GetDashDits() : GetDotDits());
      vCodeBitMask >>= 1;
      if (vCodeBitMask) {
        TSignalOffFunc(GetSymbolPauseDits());
      }
    }
  }

  static void SignalCharPause(bool aSignal) {
    if (aSignal) {
      TSignalOffFunc(GetCharPauseDits());
    }
  }

  template <typename TValue>
  static void Signal(TValue aValue, bool aPauseAtBeginAndEnd = true) {
    SignalCharPause(aPauseAtBeginAndEnd);
    if (aValue < 0) {
      aValue = -aValue;
      SignalChar('-');
      TSignalOffFunc(GetCharPauseDits());
    }
    TValue vDivisor = 1;
    while (aValue / vDivisor >= 10) {
      vDivisor *= 10;
    }
    while (vDivisor) {
      uint8_t vDigit = aValue / vDivisor;
      SignalChar('0' + vDigit);
      aValue %= vDivisor;
      vDivisor /= 10;
      if (vDivisor) {
        TSignalOffFunc(GetCharPauseDits());
      }
    };
    SignalCharPause(aPauseAtBeginAndEnd);
  }

  static void Signal(const char* aText, bool aPauseAtBeginAndEnd = true) {
    SignalCharPause((*aText != '\0') && aPauseAtBeginAndEnd);
    const char* vCharPtr = aText;
    while (*vCharPtr != '\0') {
      if (*vCharPtr != ' ') {
        SignalChar(*vCharPtr);
        ++vCharPtr;
        if (*vCharPtr != '\0' && *vCharPtr != ' ') {
          TSignalOffFunc(GetCharPauseDits());
        }
      }
      else {
        TSignalOffFunc(GetWordPauseDits());
        ++vCharPtr;
      }
    }
    SignalCharPause((*aText != '\0') && aPauseAtBeginAndEnd);
  }

};