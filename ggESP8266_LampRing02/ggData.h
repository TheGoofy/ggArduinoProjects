#pragma once

#include "ggDataEEPromT.h"
#include "ggColor.h"

typedef ggDataEEPromT<
  32,            // TStringLength
  4,             // TNumScenes
  float,         // TBrightness
  6,             // TNumBrightnesses
  ggColor::cHSV, // TColor
  4              // TNumColors
> ggData;
