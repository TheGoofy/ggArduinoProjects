#include "ggRotaryEncoder.h"
#include "ggButton.h"


#define M_BUTTON_PIN 12

ggButton mButton(M_BUTTON_PIN, true, true);

#define M_ENCODER_PIN_A 4
#define M_ENCODER_PIN_B 5

ggRotaryEncoder mEncoder(M_ENCODER_PIN_A, M_ENCODER_PIN_B);


void setup()
{
  Serial.begin(115200);
  Serial.println();

  mButton.OnChanged([] (bool aPressed) {
    Serial.printf("ButtonPressed = %d\n", aPressed);
  });
  
  mEncoder.OnValueChanged([] (long aValue) {
    Serial.printf("EncoderValue = %d\n", aValue);
  });

  mButton.Begin();
  mEncoder.Begin();
}


void loop()
{
  mButton.Run();
  mEncoder.Run();
}
