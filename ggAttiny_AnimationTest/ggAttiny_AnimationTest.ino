#include <FastLED.h>
#include "ggControlPWM.h"

#define NUM_LEDS 12     // Number of LEDs in the circle
#define DATA_PIN 8      // Pin where the LEDs are connected
#define POWER_PIN 10    // Pin to control 5V power to LEDs
#define POWER_12V_PIN 9 // Pin to control 12V power to the fan
#define FAN_PWM_PIN 6   // PWM control pin for the fan

CRGB leds[NUM_LEDS];

// Initialize PWM control for the fan
ggControlPWM<FAN_PWM_PIN, 25000> mFanPWM;

void setup() {
  pinMode(POWER_PIN, OUTPUT);          // Set pin 10 as output for 5V power control
  digitalWrite(POWER_PIN, HIGH);       // Turn on 5V power to the LEDs
  
  pinMode(POWER_12V_PIN, OUTPUT);      // Set pin 9 as output for 12V power control
  digitalWrite(POWER_12V_PIN, HIGH);   // Turn on 12V power to the fan

  mFanPWM.Begin();                     // Initialize the fan PWM control
  
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.clear();                     // Ensure all LEDs are off initially
}

void loop() {
  // Smooth animation based on the pattern with rotation
  for (int brightness = 0; brightness <= 255; brightness++) {
    FastLED.clear();

    // Define dim white color with current brightness
    CRGB dimWhite = CRGB(brightness, brightness, brightness);

    // Set the LED colors based on the rotating pattern
    for (int i = 0; i < NUM_LEDS; i++) {
      int index = (i + (millis() / 200)) % NUM_LEDS; // Slow down rotation

      if (index % 4 == 0) {
        leds[i] = dimWhite;  // Set to dim white for every fourth LED
      } else {
        leds[i] = CRGB::Red; // Set to red for the first three in each group
      }
    }

    FastLED.show();          // Update the LED strip
    delay(10);              // Adjust delay for smoothness
  }

  // Fade back to off
  for (int brightness = 255; brightness >= 0; brightness--) {
    FastLED.clear();

    CRGB dimWhite = CRGB(brightness, brightness, brightness);

    for (int i = 0; i < NUM_LEDS; i++) {
      int index = (i + (millis() / 200)) % NUM_LEDS; // Slow down rotation

      if (index % 4 == 0) {
        leds[i] = dimWhite;  // Set to dim white for every fourth LED
      } else {
        leds[i] = CRGB::Red; // Set to red for the first three in each group
      }
    }

    FastLED.show();          // Update the LED strip
    delay(10);              // Adjust delay for smoothness
  }

  // delay(500);               // Pause before restarting the animation
}




/*
#include <Adafruit_NeoPixel.h>

#define DATA_PIN       8     // Use pin 8 (PA2) on the ATtiny84a for the LED data line
#define POWER_PIN      10    // Pin 10 (PA3) to control the power supply for LEDs
#define NUM_LEDS       12    // Number of LEDs in the strip

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);

// Define a color (you can modify the RGB values to get the color you want)
uint32_t color = strip.Color(255, 0, 0);  // Red color for example

void setup() {
  pinMode(POWER_PIN, OUTPUT);   // Set pin 10 (PA3) as an output to control the power
  digitalWrite(POWER_PIN, HIGH); // Turn on the power supply and keep it on

  strip.begin();    // Initialize the LED strip
  strip.show();     // Initialize all pixels to 'off'
}

void loop() {
  // Call the function to fill the strip from bottom up
  fillUpAnimation(250);  // 100 milliseconds delay between each step
}

// This function animates the LEDs from the bottom to the top
void fillUpAnimation(int delayTime) {
  // Start by turning off all the LEDs
  strip.clear();

  // Iterate over each pair of adjacent LEDs, starting from the bottom
  for (int i = 0; i < NUM_LEDS / 2; i++) {
    // Light up the LED on the bottom half and its corresponding neighbor on the top half
    strip.setPixelColor(i, color);                          // Light up LED on one side
    strip.setPixelColor(NUM_LEDS - 1 - i, color);           // Light up the corresponding LED on the other side

    strip.show();      // Update the strip to show the changes
    delay(delayTime);  // Delay for the animation effect
  }

  // Ensure the top LED gets lit up last
  strip.setPixelColor(NUM_LEDS / 2, color);  // Light up the top center LED
  strip.show();
  delay(1000);  // Pause for 1 second before resetting the animation
}
*/