#pragma once

template <typename TBin = uint16_t, uint16_t TSize = 64>
class ggHistogramT
{
public:

  ggHistogramT() {
    Clear();
  }

  inline uint16_t GetSize() const {
    return TSize;
  }

  inline void Clear() {
    memset(mBins, 0, TSize * sizeof(TBin));  
  }

  inline void Count(int16_t aBin) {
    ++mBins[ClampBinIndex(aBin)];
  }

  const TBin& GetCount(int16_t aBin) const {
    return mBins[ClampBinIndex(aBin)];
  }

  template <typename TSerial>
  void PrintT(TSerial& aSerialT) {
    for (int16_t vBin = 0; vBin < TSize; vBin++) {
      aSerialT.print(mBins[vBin]);
      aSerialT.print(vBin < TSize - 1 ? " " : "\n");
    }    
  }

private:

  inline int16_t ClampBinIndex(int16_t aBin) const {
    return std::min<int16_t>(std::max<int16_t>(0, aBin), TSize - 1);
  }

  TBin mBins[TSize];

};
