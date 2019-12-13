#include "ggRotaryEncoder.h"


#define M_ENCODER_PIN_A 4
#define M_ENCODER_PIN_B 5

ggRotaryEncoder mEncoder(M_ENCODER_PIN_A, M_ENCODER_PIN_B);


void setup()
{
  Serial.begin(115200);
  Serial.println();
  mEncoder.OnValueChanged([] (long aValue) {
    Serial.printf("vEncoderValue = %d\n", aValue);
  });
  mEncoder.Begin();
}


void loop()
{
  mEncoder.Run();
}
