#include "ggDisplay.h"


ggDisplay mDisplay;


void setup()
{
  mDisplay.Begin();
}


void loop()
{
  static ggDisplay::tAlign vAlign = ggDisplay::eAlignLeft;
  switch (vAlign) {
    case ggDisplay::eAlignLeft: vAlign = ggDisplay::eAlignCenter; break;
    case ggDisplay::eAlignCenter: vAlign = ggDisplay::eAlignRight; break;
    case ggDisplay::eAlignRight: vAlign = ggDisplay::eAlignLeft; break;
  }

  mDisplay.SetTitle("192.168.0.255", vAlign);
  mDisplay.SetText(0,"Temperature: 21.3""\xb0""C", vAlign);
  mDisplay.SetText(1,"Humidity: 41.2%", vAlign);
  mDisplay.SetText(2,"Pressure: 870hPa", vAlign);
  mDisplay.SetText(3,"goofy was here", vAlign);

  mDisplay.Update();
  
  delay(2000);
}
