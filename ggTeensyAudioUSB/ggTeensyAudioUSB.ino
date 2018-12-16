#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthKarplusStrong  string3;        //xy=154.1999969482422,248.1999969482422
AudioSynthKarplusStrong  string2;        //xy=161.1999969482422,210.1999969482422
AudioSynthKarplusStrong  string4;        //xy=169.1999969482422,312.1999969482422
AudioSynthKarplusStrong  string1;        //xy=183.1999969482422,164.1999969482422
AudioMixer4              mixer1;         //xy=361.1999969482422,193.1999969482422
AudioMixer4              mixer2;         //xy=389.1999969482422,306.1999969482422
AudioEffectDelay         delay1;         //xy=515.2666625976562,376.26666259765625
AudioOutputUSB           usb1;           //xy=534.2000122070312,183.1999969482422
AudioOutputAnalog        dac;            //xy=547.2000122070312,265
AudioConnection          patchCord1(string3, 0, mixer1, 2);
AudioConnection          patchCord2(string3, 0, mixer2, 2);
AudioConnection          patchCord3(string2, 0, mixer1, 1);
AudioConnection          patchCord4(string2, 0, mixer2, 1);
AudioConnection          patchCord5(string4, 0, mixer1, 3);
AudioConnection          patchCord6(string4, 0, mixer2, 3);
AudioConnection          patchCord7(string1, 0, mixer1, 0);
AudioConnection          patchCord8(string1, 0, mixer2, 0);
AudioConnection          patchCord9(mixer1, 0, usb1, 0);
AudioConnection          patchCord10(mixer2, delay1);
AudioConnection          patchCord11(delay1, 0, usb1, 1);
// GUItool: end automatically generated code


void setup() {
  // put your setup code here, to run once:
  AudioMemory(25);
  delay1.delay(0, 30.0);
}

void loop() {
  // put your main code here, to run repeatedly:
  float scale = 0.8 + (float)rand()/RAND_MAX;
  string1.noteOn(scale*110.0, 1.0); delay(100);
  string2.noteOn(scale*130.8, 0.8); delay(100);
  string3.noteOn(scale*164.8, 0.6); delay(100);
  string4.noteOn(scale*220.0, 0.5); delay(700);
}
