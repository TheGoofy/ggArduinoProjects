#pragma once

#include "ggDataEEPromT.h"
#include "ggColor.h"

typedef ggDataEEPromT<
  32,            // TStringLength
  4,             // TNumScenesMax
  float,         // TBrightness
  6,             // TNumBrightnesses
  ggColor::cHSV, // TColor
  4,             // TNumColors
  12             // TNumAlarmsMax
> ggData;
