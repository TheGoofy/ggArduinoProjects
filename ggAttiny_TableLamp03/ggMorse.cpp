#include <Arduino.h>
#include "ggMorse.h"


static const char* GetCodePGM(char aChar)
{
  switch (toupper(aChar)) {
    case ' ': return PSTR(" ");
    #ifdef GG_MORSE_SUPPORT_NUMBERS
    case '0': return PSTR("-----");
    case '1': return PSTR(".----");
    case '2': return PSTR("..---");
    case '3': return PSTR("...--");
    case '4': return PSTR("....-");
    case '5': return PSTR(".....");
    case '6': return PSTR("-....");
    case '7': return PSTR("--...");
    case '8': return PSTR("---..");
    case '9': return PSTR("----.");
    #endif // GG_MORSE_SUPPORT_NUMBERS
    #ifdef GG_MORSE_SUPPORT_LETTERS
    case 'A': return PSTR(".-");
    case 'B': return PSTR("-...");
    case 'C': return PSTR("-.-.");
    case 'D': return PSTR("-..");
    case 'E': return PSTR(".");
    case 'F': return PSTR("..-.");
    case 'G': return PSTR("--.");
    case 'H': return PSTR("....");
    case 'I': return PSTR("..");
    case 'J': return PSTR(".---");
    case 'K': return PSTR("-.-");
    case 'L': return PSTR(".-..");
    case 'M': return PSTR("--");
    case 'N': return PSTR("-.");
    case 'O': return PSTR("---");
    case 'P': return PSTR(".--.");
    case 'Q': return PSTR("--.-");
    case 'R': return PSTR(".-.");
    case 'S': return PSTR("...");
    case 'T': return PSTR("-");
    case 'U': return PSTR("..-");
    case 'V': return PSTR("...-");
    case 'W': return PSTR(".--");
    case 'X': return PSTR("-..-");
    case 'Y': return PSTR("-.--");
    case 'Z': return PSTR("--..");
    #endif // GG_MORSE_SUPPORT_LETTERS
    #ifdef GG_MORSE_SUPPORT_SYMBOLS
    case '.': return PSTR(".-.-.-");
    case ',': return PSTR("--..--");
    case ':': return PSTR("---...");
    case ';': return PSTR("-.-.-.");
    case '?': return PSTR("..--..");
    case '!': return PSTR("-.-.--");
    case '-': return PSTR("-....-");
    case '_': return PSTR("..--.-");
    case '(': return PSTR("-.--.");
    case ')': return PSTR("-.--.-");
    case '\'': return PSTR(".----.");
    case '"': return PSTR(".-..-.");
    case '=': return PSTR("-...-");
    case '+': return PSTR(".-.-.");
    case '/': return PSTR("-..-.");
    case '&': return PSTR(".-...");
    case '$': return PSTR("...-..-");
    case '@': return PSTR(".--.-.");
    #endif // GG_MORSE_SUPPORT_SYMBOLS
    default: return PSTR("...---..."); // SOS
  }
}


ggMorse::ggMorse(tSignalFunc aSignalFunc,
                 tSpaceFunc aSpaceFunc,
                 long aMillisDit)
: mSignalFunc(aSignalFunc),
  mSpaceFunc(aSpaceFunc),
  mMillisDit(aMillisDit)
{
}


void ggMorse::SignalChar(const char aChar) const
{
  const char* vCodePtrPGM = GetCodePGM(aChar);
  char vCode = pgm_read_byte_near(vCodePtrPGM);
  while (vCode) {
    switch (vCode) {
      case '.': mSignalFunc(1 * mMillisDit); break;
      case '-': mSignalFunc(3 * mMillisDit); break;
      case ' ': mSpaceFunc(8 * mMillisDit, false); break;
      default: break;
    }
    mSpaceFunc(1 * mMillisDit, true);
    vCode = pgm_read_byte_near(++vCodePtrPGM);
  }
}


void ggMorse::Signal(const char* aText) const
{
  const char* vCharPtr = aText;
  while (*vCharPtr != '\0') {
    SignalChar(*vCharPtr++);
    if (*vCharPtr != '\0') {
      mSpaceFunc(5 * mMillisDit, false);
    }
  }
}
