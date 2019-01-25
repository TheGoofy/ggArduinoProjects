#include "ggDisplay.h"


ggDisplay mDisplay(10, 4);


void FillRect(int aX,
              int aY,
              int aW,
              int aH,
              unsigned char aR,
              unsigned char aG,
              unsigned char aB)
{
  for (int vY = aY; vY < aY + aH; vY++) {
    for (int vX = aX; vX < aX + aW; vX++) {
      mDisplay.SetPixelRGB(vX, vY, aR, aG, aB);
    }
  }
}


void Palette(int aCode)
{
  mDisplay.TurnOff();
  mDisplay.Clear();
  for (int vY = 0; vY < 8; vY++) {
    for (int vX = 0; vX < 8; vX++) {
      switch (aCode) {
        case 0: mDisplay.SetPixelRGB(vX+2, vY, 32*vX, 32*vY, 255-32*vX); break;
        case 1: mDisplay.SetPixelRGB(vX+2, vY, 32*vY, 255-32*vX, 32*vX); break;
        case 2: mDisplay.SetPixelRGB(vX+2, vY, 255-32*vX, 32*vX, 32*vY); break;
        default: break;
      }
    }
  }
}


void Italy()
{
  mDisplay.TurnOff();
  FillRect(0, 0, 4, 8, 200, 0, 0);
  FillRect(4, 0, 4, 8, 255, 255, 255);
  FillRect(8, 0, 4, 8, 0, 200, 0);
}


void Spain()
{
  mDisplay.TurnOff();
  FillRect(0, 0, 12, 8, 200, 0, 0);
  FillRect(0, 2, 12, 4, 255, 255, 0);
}


void Switzerland()
{
  mDisplay.TurnOff();
  mDisplay.Clear();
  FillRect(2, 0, 8, 8, 200, 0, 0);
  FillRect(3, 3, 6, 2, 255, 255, 255);
  FillRect(5, 1, 2, 6, 255, 255, 255);
}


void Iceland()
{
  mDisplay.TurnOff();
  FillRect(0, 0, 12, 8, 0, 0, 255);
  FillRect(0, 2, 12, 4, 255, 255, 255);
  FillRect(6, 0, 4, 8, 255, 255, 255);
  FillRect(0, 3, 12, 2, 200, 0, 0);
  FillRect(7, 0, 2, 8, 200, 0, 0);
}


void Sweden()
{
  mDisplay.TurnOff();
  FillRect(0, 0, 12, 8, 0, 0, 255);
  FillRect(0, 3, 12, 2, 255, 255, 0);
  FillRect(7, 0, 2, 8, 255, 255, 0);
}


void setup()
{
  mDisplay.setup();
  Switzerland();
}


void loop()
{
  static int vCode = 0;
  static long vMillisLast = 0;
  long vMillis = millis();
  if (vMillis - vMillisLast >= 2000) {
    vMillisLast = vMillis;
    vCode = (vCode + 1) % 8;
    switch (vCode) {
      case 0: Switzerland(); break;
      case 1: Sweden(); break;
      case 2: Spain(); break;
      case 3: Italy(); break;
      case 4: Iceland(); break;
      case 5: Palette(0); break;
      case 6: Palette(1); break;
      case 7: Palette(2); break;
      default: break;
    }
  }

  mDisplay.run();
}

