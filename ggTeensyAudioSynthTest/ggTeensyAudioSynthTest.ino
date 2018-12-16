#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=105.19999694824219,159.2000026702881
AudioEffectEnvelope      envelope1;      //xy=271.2000274658203,198.60001182556152
AudioOutputUSB           usb1;           //xy=430.2000274658203,189.2000026702881
AudioOutputAnalog        dac1;           //xy=453.20001220703125,326.2000026702881
AudioConnection          patchCord1(waveform1, envelope1);
AudioConnection          patchCord2(envelope1, 0, usb1, 0);
AudioConnection          patchCord3(envelope1, 0, usb1, 1);
// GUItool: end automatically generated code


#define GG_ENVELOPE_DELAY 0.0f
#define GG_ENVELOPE_ATTACK 50.0f
#define GG_ENVELOPE_HOLD 0.5f
#define GG_ENVELOPE_DECAY 50.0f
#define GG_ENVELOPE_RELEASE 50.0f

void setup()
{
  AudioMemory(25);
  envelope1.delay(GG_ENVELOPE_DELAY);
  envelope1.attack(GG_ENVELOPE_ATTACK);
  envelope1.hold(GG_ENVELOPE_HOLD);
  envelope1.decay(GG_ENVELOPE_DECAY);
  envelope1.release(GG_ENVELOPE_RELEASE);
  envelope1.sustain(0.5f);
}


void play(float aFrequency,
          float aDurationMS)
{
  envelope1.noteOff();
  delay(GG_ENVELOPE_RELEASE);
  waveform1.begin(1.0, aFrequency, WAVEFORM_TRIANGLE);
  envelope1.noteOn();
  delay(aDurationMS - GG_ENVELOPE_RELEASE);
}


void loop()
{
  play(110.0f, 250);
  play(130.8f, 250);
  play(164.8f, 250);
  play(220.0f, 250);
}
