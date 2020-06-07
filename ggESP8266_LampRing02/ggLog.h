#pragma once

#include <vector>

#include "ggNumerics.h"

class ggLog {

public:

  ggLog(int aMax) {
    InitializeTable(aMax);
  }

  inline int Get(const float& aInputValue) const {
    int vIndex = aInputValue * (mTable.size() - 1) + 0.5f;
    return mTable[ggClamp<int>(vIndex, 0, mTable.size() - 1)];
  }

private:

  void InitializeTable(int aMax) {
    int vIndex = 0;
    int vStep = 1;
    while (vIndex < aMax) {
      mTable.push_back(vIndex);
      while (vIndex / vStep > 40) vStep++;
      vIndex += vStep;
    }
    mTable.push_back(aMax);
  }

  std::vector<int> mTable;
  
};
