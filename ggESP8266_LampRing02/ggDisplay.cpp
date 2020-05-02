#include "ggDisplay.h"


void ggDisplay::PrintDebug(const String& aName) const 
{
  ggDebug vDebug("ggDisplay", aName);
  vDebug.PrintF("GetAddressI2C() => 0x%02X\n", GetAddressI2C());
  vDebug.PrintF("mConnected = %d\n", mConnected);
  vDebug.PrintF("mCursorY = %d\n", mCursorY);
  vDebug.PrintF("mUpdate = %d\n", mUpdate);
  mTitle.PrintDebug("mTitle");
  for (uint32_t vIndex = 0; vIndex < mText.size(); vIndex++) {
    mText[vIndex].PrintDebug("mText[" + String(vIndex) + "]");
  }
}

void ggDisplay::cText::PrintDebug(const String& aName) const
{
  ggDebug vDebug("ggDisplay::cText", aName);
  vDebug.PrintF("mString = \"%s\"\n", mString.c_str());
  vDebug.PrintF("mAlign = %s\n", ToString(mAlign).c_str());
}


template <>
String ToString(const ggDisplay::cAlign::tEnum& aAlign)
{
  switch (aAlign) {
    case ggDisplay::cAlign::eLeft: return "cAlign::eLeft";
    case ggDisplay::cAlign::eCenter: return "cAlign::eCenter";
    case ggDisplay::cAlign::eRight: return "cAlign::eRight";
    default: return "eAlignUnknown";
  }
}


template <>
ggDisplay::cAlign::tEnum FromString(const String& aString)
{
  if (aString == "cAlign::eLeft") return ggDisplay::cAlign::eLeft;
  if (aString == "cAlign::eCenter") return ggDisplay::cAlign::eCenter;
  if (aString == "cAlign::eRight") return ggDisplay::cAlign::eRight;
  return ggDisplay::cAlign::eLeft;
}
