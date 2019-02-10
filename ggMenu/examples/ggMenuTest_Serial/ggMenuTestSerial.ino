#include "ggVector.h"
#include "ggMenu.h"

#include "ggDemoMenu.h"

void setup()
{
  Serial.begin(115200);
  Serial.println();
  mDemoMenu.print(Serial);
}

void loop()
{
  if (Serial.available()) {
    
    char vChar = Serial.read();
    switch (vChar) {
      case '6': mDemoMenu.SelectEnter(); break;
      case '4': mDemoMenu.SelectExit(); break;
      case '8': mDemoMenu.SelectNext(); break;
      case '2': mDemoMenu.SelectPrev(); break;
      default: break;
    }
    
    Serial.print(mDemoMenu.IsEditing() ? "*" : ">"); 
    mDemoMenu.PrintSibling(Serial, mDemoMenu.GetItemSelectedIndex());
    Serial.println();
  }
}


