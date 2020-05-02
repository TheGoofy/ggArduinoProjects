#include "ggRotaryEncoder.h"

int ggRotaryEncoder::mPinA = 0;
int ggRotaryEncoder::mPinB = 0;

volatile bool ggRotaryEncoder::mBitA = false;
volatile bool ggRotaryEncoder::mBitB = false;
volatile long ggRotaryEncoder::mValue = 0;
volatile long ggRotaryEncoder::mValueLast = 0;
