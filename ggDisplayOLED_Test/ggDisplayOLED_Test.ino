#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define GG_VOLTAGE_PIN A3


int GetTextSizeX(int aValue, int aTextSize)
{
  int vChars = 0;
  if      (aValue <= -1000) vChars = 5;
  else if (aValue <= -100) vChars = 4;
  else if (aValue <= -10) vChars = 3;
  else if (aValue <= -1) vChars = 2;
  else if (aValue < 10) vChars = 1;
  else if (aValue < 100) vChars = 2;
  else if (aValue < 1000) vChars = 3;
  else if (aValue < 10000) vChars = 4;
  else vChars = 5;
  return vChars * 6 * aTextSize;
}


int GetTextSizeX(float aValue, int aDecimals, int aTextSize)
{
  int vValue = (int)aValue;
  int vChars = 0;
  if      (vValue <= -1000) vChars = 5;
  else if (vValue <= -100) vChars = 4;
  else if (vValue <= -10) vChars = 3;
  else if (vValue <= -1) vChars = 2;
  else if (vValue < 10) vChars = 1;
  else if (vValue < 100) vChars = 2;
  else if (vValue < 1000) vChars = 3;
  else if (vValue < 10000) vChars = 4;
  else vChars = 5;
  if (aDecimals > 0) vChars += aDecimals + 1;
  return 6 * vChars * aTextSize;
}


void DrawBattery(Adafruit_SSD1306& aDisplay,
                 int aPosX, int aPosY,
                 int aSizeX, int aSizeY,
                 int aPercent,
                 float aVoltage)
{ 
  const int vGap = 2;
  const int vCap = 3;
  aSizeX -= vCap;
  aDisplay.drawRoundRect(aPosX, aPosY, aSizeX, aSizeY, 3, WHITE);
  aDisplay.fillRect(aPosX+aSizeX, aPosY+aSizeY/4, vCap, aSizeY/2, WHITE);
  int vPercent = (aPercent < 0) ? (0) : (aPercent < 100 ? aPercent : 100);
  aDisplay.fillRoundRect(aPosX+vGap, aPosY+vGap, vPercent*(aSizeX-2*vGap)/100, aSizeY-2*vGap, 1, WHITE);
  int vTextSize = 2; //(aSizeY-2*vGap)/9;
  if (vTextSize > 0) {
    aDisplay.setTextSize(vTextSize);
    aDisplay.setTextColor(INVERSE);
    /*
    if (aPercent > 50) aDisplay.setCursor(aPosX+2*vGap, aPosY+2*vGap);
    else aDisplay.setCursor(aPosX+aSizeX-2*vGap-GetTextSizeX(aPercent,vTextSize)-5*vTextSize, aPosY+2*vGap);
    aDisplay.print(aPercent);
    aDisplay.print("%");
    */
    const int vDecimals = 1;
    if (aPercent > 50) aDisplay.setCursor(aPosX+2*vGap, aPosY+2*vGap);
    else aDisplay.setCursor(aPosX+aSizeX-2*vGap-GetTextSizeX(aVoltage,vDecimals,vTextSize)-5*vTextSize, aPosY+2*vGap);
    aDisplay.print(aVoltage,vDecimals);
    aDisplay.print("V");
  }
}


void PrintVoltage()
{
  int vValue = analogRead(GG_VOLTAGE_PIN);
  const float vFilter = 0.95f;
  static float vVoltage = 0.0f;
  vVoltage = vFilter*vVoltage + (1.0f-vFilter)*(0.0192f*vValue);

  const float vVoltageMin = 3.0f * 3.0f;
  const float vVoltageMax = 3.0f * 4.2f;
  int vPercent = (int)(100.0f * (vVoltage - vVoltageMin) / (vVoltageMax - vVoltageMin));
  
  display.clearDisplay();


  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(vValue);
  display.print(vVoltage, 1);
  display.println("V");


  const int vBatterySizeY = 22;
  DrawBattery(display, 0, 64-vBatterySizeY, 128, vBatterySizeY, vPercent, vVoltage); 
  
  display.display();
}


void setup()
{
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.clearDisplay();
}


void loop()
{
  PrintVoltage();
  delay(50);
}
